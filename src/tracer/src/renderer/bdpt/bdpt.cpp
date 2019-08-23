#include <atomic>
#include <mutex>
#include <queue>
#include <random>
#include <thread>

#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/reporter.h>
#include <agz/tracer/core/renderer.h>
#include <agz/tracer/core/sampler.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/utility/grid_divider.h>
#include <agz/utility/thread.h>

AGZ_TRACER_BEGIN

namespace bdpt
{

class BDPTEndPoint
{
public:

    Vec3 pos;
    Vec3 nor;
    real pdf_area = 0;

    Spectrum f;
    real pdf_dir  = 0;
    bool is_delta = false;

    virtual ~BDPTEndPoint() = default;

    // return (emission, pdf)
    virtual std::pair<Spectrum, real> eval(const Vec3 &next_pos) const noexcept = 0;
};

class Vertex
{
public:

    Vec3 pos;
    Vec3 wr;
    Vec3 wsam;

    Spectrum accumulated_f; // bsdf(wsam, wr)

    real pdf_proj_radiance   = 0;
    real pdf_proj_importance = 0;
    bool is_delta            = false;
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

        int cam_min_depth = 4;
        int lht_min_depth = 4;

        int cam_max_depth = 10;
        int lht_max_depth = 10;

        real cam_cont_prob = real(0.9);
        real lht_cont_prob = real(0.9);

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

    Spectrum eval(const Ray &cam_ray, Sampler &sampler, GBufferPixel *gpixel, Arena &arena, FilmGrid *full_film_grid, const Vec2i &full_res)
    {
        // TODO
        return {};
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
        auto f = eval(cam_ray.r, sampler, &gpixel, arena, full_film_grid, full_res);

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

        params_.cam_min_depth = params.child_int_or("cam_min_depth", params_.cam_min_depth);
        if(params_.cam_min_depth < 1)
            throw ObjectConstructionException("invalid cam_min_depth value: " + std::to_string(params_.cam_min_depth));

        params_.cam_max_depth = params.child_int_or("cam_max_depth", params_.cam_max_depth);
        if(params_.cam_max_depth < params_.cam_min_depth)
            throw ObjectConstructionException("invalid cam_max_depth value: " + std::to_string(params_.cam_max_depth));

        params_.lht_min_depth = params.child_int_or("lht_min_depth", params_.lht_min_depth);
        if(params_.lht_min_depth < 1)
            throw ObjectConstructionException("invalid lht_min_depth value: " + std::to_string(params_.lht_min_depth));

        params_.lht_max_depth = params.child_int_or("lht_max_depth", params_.lht_max_depth);
        if(params_.lht_max_depth < params_.lht_min_depth)
            throw ObjectConstructionException("invalid lht_max_depth value: " + std::to_string(params_.lht_max_depth));

        params_.cam_cont_prob = params.child_real_or("cam_cont_prob", params_.cam_cont_prob);
        if(params_.cam_cont_prob < 0 || params_.cam_cont_prob > 1)
            throw ObjectConstructionException("invalid cam_cont_prob value: " + std::to_string(params_.cam_cont_prob));

        params_.lht_cont_prob = params.child_real_or("lht_cont_prob", params_.lht_cont_prob);
        if(params_.lht_cont_prob < 0 || params_.lht_cont_prob > 1)
            throw ObjectConstructionException("invalid lht_cont_prob value: " + std::to_string(params_.lht_cont_prob));

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
