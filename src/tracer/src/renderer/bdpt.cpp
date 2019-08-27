#include <atomic>
#include <mutex>
#include <queue>
#include <random>
#include <thread>

#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/reporter.h>
#include <agz/tracer/core/renderer.h>
#include <agz/tracer/core/sampler.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/utility/grid_divider.h>
#include <agz/utility/thread.h>

/*
generate cam subpath c1, c2, ..., cs
generate lht subpath l1, l2, ..., lt

ret = black

for ci = 1 to cs
    for li = 0 to lt

        // path长度
        k = ci + li

        if k < 2
            continue
        if k == 2
            // 对len == 2的情况单独处理：只算cam ray的contribution，不使用cam sampling，当然也就谈不上mis
            if ci == 2
                ret += unweighted contribution
            continue

        // 构建一条contribution不为0的路径
        if ci == 1
            p = sample camera to construct a path
            if p is out of film
                continue
            use_cam_sam = true
            recompute we
        else if li == 0
            p = c1...ci
            if ci is not a light source
                continue
            use_cam_sam = false
            recompute le
        else if li == 1
            p = c1...ci li
            use_cam_sam = false
            recompute le
        else
            p = c1...ci, li...l1
            if ci and li are not visible
                continue
            use_cam_sam = false

        throughput = we * ... * bsdf * G * bsdf * ... * le

        cur_pdf = (cam.pdf / cam.cos) * (pdf / cos) * ... * (lht.pdf / lht.cos)
        wht_dem = 1

        tpdf = 1
        for i = ci; i >= 2; --i
            tpdf *= (pdf{i+1 to i} / cos) / (pdf{i-1 to i} / cos)
            tpdf *= G(i, i+1) / G(i-1, i)
            if i-1 is not specular
                wht_dem += tpdf * tpdf

        tpdf = 1
        for i = ci+1; i <= k-1; ++i
            tpdf *= (pdf{i-1 to i} / cos) / (pdf{i+1 to i} / cos)
            tpdf *= G(i-1, i) / G(i, i+1)
            if i+1 is not specular
                wht_dem += tpdf * tpdf

        if li >= 1
            tpdf *= (pdf{k-1 to k} / cos) / lht.pdf_area
            tpdf *= G(k-2, k-1)
            wht_dem += tpdf * tpdf

        mis_weight = 1 / wht_dem
        contrib = mis_weight * throughput / cur_pdf

        if use_cam_sam
            add contrib to full_film_grid
        else
            ret += contrib
*/

AGZ_TRACER_BEGIN

namespace bdpt
{

struct Vertex
{
    Vec3 pos;
    Vec3 nor;

    const BSDF *bsdf = nullptr;
    bool is_specular = false;

    Spectrum accu_f; // 从cam we开始，一直到这个顶点，中间所有的bsdf值的乘积；注意该顶点的bsdf也算进去
                              // 对subpath末端的顶点而言，该成员无意义

    real pdf_right = -1; // 该顶点向lht方向被采样到的pdf，也就是p(i to i+1)存放在i.pdf_right中
    real pdf_left  = -1; // 该顶点向cam方向被采样到的pdf，也就是p(i+1 to i)存放在i.pdf_left中

    const Entity *entity = nullptr;
};

class BDPT : public Renderer
{
    using GridDivider = GridDivider<int>;
    using Grid        = GridDivider::Grid;

    std::mutex full_grid_mutex_;
    std::unordered_map<std::thread::id, std::shared_ptr<FilmGrid>> thread_2_full_grids_;

    const Sampler *sampler_prototype_ = nullptr;
    std::mutex sampler_prototype_mutex_;

    struct Input
    {
        const Camera           *camera   = nullptr;
        const Scene            *scene    = nullptr;

        std::mutex reporter_mutex;
        ProgressReporter *reporter = nullptr;

        std::mutex film_mutex;
        Film *film = nullptr;

        void clear() noexcept
        {
            camera   = nullptr;
            scene    = nullptr;
            reporter = nullptr;
            film     = nullptr;
        }

    } input_;

    struct Params
    {
        int task_grid_size = 24;
        int worker_count   = -1;

        int cam_max_depth = 10;
        int lht_max_depth = 10;

    } params_;

    struct Statistics
    {
        std::atomic<int> N = 0;

        void clear() noexcept
        {
            N = 0;
        }

    } statistics;

    /** 取得本线程独有的full res film grid；每个线程第一次访问时都会新创建一个 */
    FilmGrid *get_full_film_grid()
    {
        auto id = std::this_thread::get_id();

        {
            std::lock_guard lk(full_grid_mutex_);
            auto it = thread_2_full_grids_.find(id);
            if(it != thread_2_full_grids_.end())
                return it->second.get();
        }

        std::unique_ptr<FilmGrid> grid;
        {
            std::lock_guard lk(input_.film_mutex);
            auto [w, h] = input_.film->resolution();
            grid = input_.film->new_grid(0, w, 0, h);
        }

        auto ret = grid.get();
        std::lock_guard lk(full_grid_mutex_);
        thread_2_full_grids_[id] = std::move(grid);

        return ret;
    }

    static void gen_subpath(
        Ray r, const Scene &scene, Sampler &sampler,
        const Spectrum &init_accu_f, real init_pdf,
        std::vector<Vertex> *output, GBufferPixel *gpixel,
        TransportMode mode, Arena &arena, int max_subpath_depth)
    {
        Spectrum accu_f = init_accu_f;
        real last_to_this_pdf = init_pdf;

        for(int depth = 1; depth <= max_subpath_depth; ++depth)
        {
            EntityIntersection inct;
            if(!scene.closest_intersection(r, &inct))
                break;

            auto shd = inct.material->shade(inct, arena);
            if(depth == 1 && gpixel)
            {
                gpixel->albedo   = shd.bsdf->albedo();
                gpixel->depth    = r.d.length() * inct.t;
                gpixel->position = inct.pos;
                gpixel->normal   = inct.user_coord.z;
            }

            auto bsdf_sample = shd.bsdf->sample(inct.wr, mode, sampler.sample3());
            accu_f *= bsdf_sample.f;

            Vertex vtx;
            vtx.pos         = inct.pos;
            vtx.nor         = inct.geometry_coord.z;
            vtx.bsdf        = shd.bsdf;
            vtx.is_specular = shd.bsdf->is_delta();
            vtx.accu_f      = accu_f;
            vtx.entity      = inct.entity;
            if(mode == TM_Radiance)
            {
                vtx.pdf_right    = last_to_this_pdf;
                vtx.pdf_left     = -1;
                last_to_this_pdf = bsdf_sample.pdf;
            }
            else
            {
                vtx.pdf_right    = -1;
                vtx.pdf_left     = last_to_this_pdf;
                last_to_this_pdf = bsdf_sample.pdf;
            }
            output->push_back(vtx);

            if(!accu_f)
                break;
        }
    }

    Spectrum eval(const CameraGenerateRayResult &cam_ray, Sampler &sampler, GBufferPixel *gpixel,
                  Arena &arena, FilmGrid *full_film_grid, const Vec2i &full_res)
    {
        auto &scene = *input_.scene;
        Spectrum ret;

        // sample light source

        auto [light, select_light_pdf] = scene.sample_light(sampler.sample1());
        if(!light)
            return Spectrum();

        auto emit_ret = light->emit(sampler.sample5());
        if(!emit_ret.radiance)
            return Spectrum();

        // generate light subpath

        // lht_subpath[0]是个dummy vertex
        std::vector<Vertex> lht_subpath(2);

        // lht_subpath[1]是light vertex
        auto &lht_vtx = lht_subpath.back();
        lht_vtx.pos      = emit_ret.spt.pos;
        lht_vtx.nor      = emit_ret.spt.geometry_coord.z;
        lht_vtx.accu_f   = emit_ret.radiance;
        lht_vtx.pdf_left = emit_ret.pdf_pos;

        Ray emit_ray(emit_ret.spt.pos, emit_ret.dir.normalize(), EPS);
        gen_subpath(emit_ray, scene, sampler, emit_ret.radiance, emit_ret.pdf_dir,
                    &lht_subpath, nullptr, TM_Importance, arena, params_.lht_max_depth);

        // TODO
        //std::vector<Vertex> lht_subpath = generate_subpath(
        //    Ray(emit_ret.spt.pos, emit_ret.dir.normalize(), EPS),
        //    scene, emit_ret.radiance, sampler, nullptr, arena, params_.lht_max_depth);

        // generate camera subpath

        // TODO
        //std::vector<Vertex> cam_subpath = generate_subpath(
        //    cam_ray.r, scene, Spectrum(cam_ray.importance), sampler, gpixel, arena, params_.cam_max_depth);

        // connect subpaths

        // TODO

        return ret;
    }

    void render_pixel_once(int px, int py, Sampler &sampler, FilmGrid *film_grid, FilmGrid *full_film_grid, const Vec2i &full_res, Arena &arena)
    {
        auto film_sam = sampler.sample2();
        real pixel_x = px + film_sam.u;
        real pixel_y = py + film_sam.v;
        real film_cube_x = real(pixel_x) / full_res.x;
        real film_cube_y = real(pixel_y) / full_res.y;

        CameraSample cam_sam = { { film_cube_x, film_cube_y }, sampler.sample2() };
        auto cam_ray = input_.camera->generate_ray(cam_sam);

        real we_cos = std::abs(cos(cam_ray.r.d, cam_ray.nor));
        real we_factor = we_cos * cam_ray.importance / (cam_ray.pdf_pos * cam_ray.pdf_dir);

        GBufferPixel gpixel;
        auto f = eval(cam_ray, sampler, &gpixel, arena, full_film_grid, full_res);

        film_grid->add_sample({ pixel_x, pixel_y }, we_factor * f, gpixel, 1);
        ++statistics.N;
    }

    void render_grid(const Grid &grid, FilmGrid *film_grid, FilmGrid *full_film_grid)
    {
        Sampler *sampler;
        Arena sampler_arena;
        {
            int new_seed;
            std::seed_seq seed_gen{ grid.x_begin, grid.y_begin };
            seed_gen.generate(&new_seed, &new_seed + 1);

            std::lock_guard lk(sampler_prototype_mutex_);
            sampler = sampler_prototype_->clone(new_seed, sampler_arena);
        }

        auto full_res = input_.film->resolution();

        int x_beg = film_grid->sample_x_beg();
        int x_end = film_grid->sample_x_end();
        int y_beg = film_grid->sample_y_beg();
        int y_end = film_grid->sample_y_end();

        Arena arena;

        for(int py = y_beg; py < y_end; ++py)
        {
            for(int px = x_beg; px < x_end; ++px)
            {
                sampler->start_pixel(px, py);
                do
                {
                    render_pixel_once(px, py, *sampler, film_grid, full_film_grid, full_res, arena);

                    if(!(px % 4))
                        arena.release();

                } while(sampler->next_sample());
            }
        }
    }

public:

    using Renderer::Renderer;

    static std::string description()
    {
        return "";
    }

    void initialize(const Config &params, obj::ObjectInitContext&) override
    {
        AGZ_HIERARCHY_TRY

        params_ = Params();
        params_.task_grid_size = params.child_int_or("task_grid_size", params_.task_grid_size);
        if(params_.task_grid_size <= 0)
            throw ObjectConstructionException("invalid task_grid_size value: " + std::to_string(params_.task_grid_size));

        params_.worker_count = params.child_int_or("worker_count", params_.worker_count);

        params_.cam_max_depth = params.child_int_or("cam_max_depth", params_.cam_max_depth);
        if(params_.cam_max_depth < 1)
            throw ObjectConstructionException("invalid cam_max_depth value: " + std::to_string(params_.cam_max_depth));

        params_.lht_max_depth = params.child_int_or("lht_max_depth", params_.lht_max_depth);
        if(params_.lht_max_depth < 1)
            throw ObjectConstructionException("invalid lht_max_depth value: " + std::to_string(params_.lht_max_depth));

        AGZ_HIERARCHY_WRAP("in initializing bdpt renderer")
    }

    void render(Scene &scene, ProgressReporter &reporter, Film *film) override
    {
        input_.camera   = scene.camera();
        input_.scene    = &scene;
        input_.reporter = &reporter;
        input_.film     = film;
        statistics.clear();

        auto full_res = film->resolution();
        std::queue<Grid> tasks;

        Grid full_grid = { 0, full_res.x, 0, full_res.y };
        GridDivider::divide(full_grid, params_.task_grid_size, params_.task_grid_size, misc::push_inserter(tasks));

        size_t total_task_count = tasks.size();
        std::atomic<size_t> finished_task_count = 0;

        auto func = [&input = input_,
                     &renderer = *this,
                     &finished_task_count, total_task_count]
            (const Grid &grid)
        {
            auto full_film_grid = renderer.get_full_film_grid();
            auto film_grid = input.film->new_grid(grid.x_begin, grid.x_end, grid.y_begin, grid.y_end);
            renderer.render_grid(grid, film_grid.get(), full_film_grid);

            {
                std::lock_guard lk(input.film_mutex);
                input.film->merge_grid(std::move(*film_grid));
            }

            double percent = 100.0 * ++finished_task_count / total_task_count;
            std::lock_guard lk(input.reporter_mutex);
            input.reporter->progress(percent);
        };

        thread::queue_executer_t<Grid> executor;

        reporter.begin();
        reporter.new_stage();

        executor.run_async(params_.worker_count, std::move(tasks), func);
        executor.join();

        reporter.end_stage();
        reporter.end();

        {
            real ratio = full_res.product() / real(statistics.N);
            for(auto &p : thread_2_full_grids_)
                film->add_grid(std::move(*p.second), Spectrum(ratio));
        }

        auto exceptions = executor.exceptions();
        for(auto &ptr : exceptions)
            reporter.message("Exception: " + misc::extract_exception_ptr(ptr));

        input_.clear();
        statistics.clear();
    }
};

} // namespace bdpt

AGZ_TRACER_END
