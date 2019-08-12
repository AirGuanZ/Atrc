#include <atomic>
#include <iterator>
#include <unordered_map>
#include <vector>

#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/renderer.h>
#include <agz/tracer/core/reporter.h>
#include <agz/tracer/core/sampler.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/utility/grid_divider.h>
#include <agz/utility/thread.h>

#include "./vp_searcher.h"

AGZ_TRACER_BEGIN

namespace sppm
{

// SPPM Renderer
// See https://www.ci.i.u-tokyo.ac.jp/~hachisuka/sppm.pdf

class SPPMRenderer : public Renderer
{
    real init_radius_ = 1;
    int iteration_count_ = 100;
    int iteration_photon_count_ = 100;

    int max_cam_depth_ = 20;

    int min_photon_depth_ = 5;
    int max_photon_depth_ = 10;
    real photon_cont_prob_ = real(0.9);

    real alpha_ = real(0.666666);

    int worker_count_ = 0;

    const Sampler *sampler_prototype_ = nullptr;
    mutable std::mutex sampler_prototype_mutex_;

    using PixelImage  = texture::texture2d_t<PixelParams>;
    using GridDivider = GridDivider<int>;
    using Grid        = GridDivider::Grid;

    Sampler *clone_sampler(int seed, Arena &arena) const
    {
        std::lock_guard lk(sampler_prototype_mutex_);
        return sampler_prototype_->clone(seed, arena);
    }

    Arena *get_threadlocal_arena(std::unordered_map<std::thread::id, std::unique_ptr<Arena>> &arenas)
    {
        auto id = std::this_thread::get_id();
        auto it = arenas.find(id);
        if(it != arenas.end())
            return it->second.get();

        auto new_arena = std::make_unique<Arena>();
        Arena *ret = new_arena.get();
        arenas[id] = std::move(new_arena);
        return ret;
    }

    Sampler *clone_sampler(int iterate_idx, const Grid &grid, Arena &arena)
    {
        size_t hash = misc::hash(iterate_idx, grid.x_begin, grid.x_end, grid.y_begin, grid.y_end);
        int new_seed = static_cast<int>(hash);
        std::lock_guard lk(sampler_prototype_mutex_);
        return sampler_prototype_->clone(new_seed, arena);
    }

    void find_visible_point(const Scene &scene, const Vec2i &res, const Vec2i &p, Sampler &sampler,
                            VisiblePoint &vp, PixelParams &pixel, GBufferPixel &gpixel, Arena &arena)
    {
        // generate camera ray

        auto film_sam = sampler.sample2();
        Sample2 film_coord{
            (p.x + film_sam.u) / res.x,
            (p.y + film_sam.v) / res.y
        };
        CameraSample cam_sam = { film_coord, sampler.sample2() };
        auto cam_ray = scene.camera()->generate_ray(cam_sam);
        
        Spectrum coef = Spectrum(cam_ray.importance / (cam_ray.pdf_pos * cam_ray.pdf_dir));
        Ray r = cam_ray.r;

        // trace camera subpath

        for(int depth = 1; depth <= max_cam_depth_; ++depth)
        {
            EntityIntersection inct;
            if(!scene.closest_intersection(r, &inct))
            {
                gpixel.depth = -1;
                return;
            }

            ShadingPoint shd = inct.material->shade(inct, arena);
            if(depth == 1)
            {
                gpixel.albedo   = shd.bsdf->albedo();
                gpixel.position = inct.pos;
                gpixel.normal   = inct.user_coord.z;
                gpixel.depth    = r.d.length() * inct.t;
            }

            if(auto light = inct.entity->as_light())
                pixel.direct_illu += coef * light->radiance(inct, inct.wr);

            // no need to fill vp with pure black bsdf
            if(shd.bsdf->is_black())
                return;

            if(!shd.bsdf->is_delta())
            {
                vp.bsdf = shd.bsdf;
                vp.pos  = inct.pos;
                vp.f    = coef;
                vp.nor  = inct.user_coord.z;
                vp.dir  = inct.wr;
                return;
            }

            auto bsdf_sample = shd.bsdf->sample(inct.wr, TM_Radiance, sampler.sample3());
            if(!bsdf_sample.f || bsdf_sample.pdf < EPS)
                return;
            coef *= bsdf_sample.f * shd.bsdf->proj_wi_factor(bsdf_sample.dir) / bsdf_sample.pdf;
            r = Ray(inct.pos, bsdf_sample.dir.normalize(), EPS);
        }
    }

    static void atomic_real_add(std::atomic<real> &var, real val)
    {
        auto current = var.load();
        while(!var.compare_exchange_weak(current, current + val));
    }

    void trace_single_photon(const Scene &scene, Sampler &sampler, VPSearcher &vps, Arena &arena)
    {
        auto [light, light_select_pdf] = scene.sample_light(sampler.sample1());
        if(!light || light_select_pdf < EPS)
            return;
        
        auto emit_result = light->emit(sampler.sample5());
        if(!emit_result.radiance)
            return;

        Spectrum coef = emit_result.radiance * std::abs(cos(emit_result.spt.geometry_coord.z, emit_result.dir))
                       / (light_select_pdf * emit_result.pdf_pos * emit_result.pdf_dir);
        Ray r(emit_result.spt.pos, emit_result.dir.normalize(), EPS);

        for(int depth = 1; depth <= max_photon_depth_; ++depth)
        {
            if(depth >= min_photon_depth_)
            {
                if(sampler.sample1().u > photon_cont_prob_)
                    return;
                coef /= photon_cont_prob_;
            }

            EntityIntersection inct;
            if(!scene.closest_intersection(r, &inct))
                return;

            vps.foreach_vp_contains(inct.pos, [&](VPRec &vp)
            {
                ++vp.pp->M;

                Spectrum f = vp.vp->f * coef * vp.vp->bsdf->eval(inct.wr, vp.vp->dir, TM_Radiance);
                atomic_real_add(vp.pp->Phi[0], f.r);
                atomic_real_add(vp.pp->Phi[1], f.g);
                atomic_real_add(vp.pp->Phi[2], f.b);
            });

            auto shd = inct.material->shade(inct, arena);
            auto bsdf_sample = shd.bsdf->sample(inct.wr, TM_Importance, sampler.sample3());
            if(!bsdf_sample.f || bsdf_sample.pdf < EPS)
                return;

            coef *= bsdf_sample.f * shd.bsdf->proj_wi_factor(bsdf_sample.dir) / bsdf_sample.pdf;
            r = Ray(inct.pos, bsdf_sample.dir.normalize(), EPS);
        }
    }

    void perform_iteration(int iterate_idx, const Scene &scene, const std::vector<Grid> &grids, PixelImage &pixel_img, GBuffer &gbuffer)
    {
        // longlive threadlocal arena for BSDF allocation in this iteration

        std::mutex threadlocal_arena_mutex;
        std::unordered_map<std::thread::id, std::unique_ptr<Arena>> threadlocal_arenas;

        // find visible points; fill gbuffer

        Vec2i res = { pixel_img.width(), pixel_img.height() };
        texture::texture2d_t<VisiblePoint> vp_img(res.y, res.x);

        thread::parallel_foreach(grids, [&](const Grid &grid)
        {
            Arena *l_arena;
            {
                std::lock_guard lk(threadlocal_arena_mutex);
                l_arena = get_threadlocal_arena(threadlocal_arenas);
            }

            // grid local sampler
            Arena s_arena;
            Sampler *sampler = clone_sampler(iterate_idx, grid, s_arena);

            // traversal all pixels in grid
            for(int y = grid.y_begin; y < grid.y_end; ++y)
            {
                for(int x = grid.x_begin; x < grid.x_end; ++x)
                {
                    sampler->start_pixel(x, y);

                    GBufferPixel gpixel;
                    find_visible_point(scene, res, { x, y }, *sampler, vp_img(y, x), pixel_img(y, x), gpixel, *l_arena);
                    
                    gbuffer.albedo  ->at(y, x) += gpixel.albedo;
                    gbuffer.position->at(y, x) += gpixel.position;
                    gbuffer.normal  ->at(y, x) += gpixel.normal;
                    gbuffer.depth   ->at(y, x) += gpixel.depth;
                }
            }
        }, worker_count_);

        // compute bound of all visible points

        AABB world_bound;
        real max_radius = 0;
        for(int y = 0; y < vp_img.height(); ++y)
        {
            for(int x = 0; x < vp_img.width(); ++x)
            {
                auto &vp = vp_img(y, x);
                if(vp.bsdf)
                {
                    auto &r = pixel_img(y, x).R;
                    max_radius = (std::max)(max_radius, r);
                    AABB vp_bound{ vp.pos - Vec3(r), vp.pos + Vec3(r) };
                    world_bound |= vp_bound;
                }
            }
        }

        // no valid visible points? emm...
        if(!world_bound.valid())
            return;

        // build spatial vp searcher

        Vec3i grid_res;
        Vec3 world_diag = world_bound.high - world_bound.low;
        for(int i = 0; i < 3; ++i)
            grid_res[i] = (std::max)(1, static_cast<int>(world_diag[i] / max_radius) / 2);

        VPSearcher vp_searcher(world_bound, grid_res);
        for(int y = 0; y < vp_img.height(); ++y)
        {
            for(int x = 0; x < vp_img.width(); ++x)
            {
                auto &vp = vp_img(y, x);
                if(vp.bsdf)
                    vp_searcher.add_vp(VPRec{ vp.pos, &vp, &pixel_img(y, x) });
            }
        }

        // eventually! photon tracing!

        int expected_task_count = thread::actual_worker_count(worker_count_) * 16;
        int task_size           = (std::max)(iteration_photon_count_ / expected_task_count, 100);
        int task_count          = (iteration_photon_count_ + task_size - 1) / task_size;

        thread::parallel_forrange(0, task_count, [&](int task_id)
        {
            Arena arena;
            Sampler *sampler;
            {
                std::lock_guard lk(sampler_prototype_mutex_);
                sampler = sampler_prototype_->clone(iterate_idx * iteration_photon_count_ + task_id, arena);
            }

            int photon_beg = task_id * task_size;
            int photon_end = (std::min)(photon_beg + task_size, iteration_photon_count_);
            for(int i = photon_beg; i < photon_end; ++i)
            {
                sampler->start_pixel(task_id, i);
                trace_single_photon(scene, *sampler, vp_searcher, arena);
            }

        }, worker_count_);

        // update pixel parameters

        for(int y = 0; y < pixel_img.height(); ++y)
        {
            for(int x = 0; x < pixel_img.width(); ++x)
                pixel_img(y, x).iterate(alpha_);
        }
    }

public:

    using Renderer::Renderer;

    static std::string description()
    {
        return R"___(
sppm [Renderer]
    init_radius            [real]    initial merge radius
    iteration_count        [int]     tracing & gathering iteration count, i.e. spp
    iteration_photon_count [int]     photon count per iteration
    max_cam_depth          [int]     maximum camera subpath depth
    min_photon_depth       [int]     minimum photon tracing depth before RR
    max_photon_depth       [int]     maximum photon tracing depth
    photon_cont_prob       [int]     RR probability in photon tracing
    alpha                  [real]    iteration parameter alpha
    worker_count           [int]     working thread count; see agz::thread::parallel_foreach
    sampler                [Sampler] sampler prototype
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        init_radius_ = params.child_real("init_radius");
        if(init_radius_ <= 0)
            throw ObjectConstructionException("invalid initial merge radius value: " + std::to_string(init_radius_));

        iteration_count_ = params.child_int("iteration_count");
        if(iteration_count_ <= 0)
            throw ObjectConstructionException("invalid iteration count value: " + std::to_string(iteration_count_));

        iteration_photon_count_ = params.child_int("iteration_photon_count");
        if(iteration_photon_count_ <= 0)
            throw ObjectConstructionException("invalid iteration photon count: " + std::to_string(iteration_photon_count_));

        max_cam_depth_ = params.child_int("max_cam_depth");
        if(max_cam_depth_ < 1)
            throw ObjectConstructionException("invalid max_cam_depth value: " + std::to_string(max_cam_depth_));

        min_photon_depth_ = params.child_int("min_photon_depth");
        if(min_photon_depth_ < 1)
            throw ObjectConstructionException("invalid min_photon_depth value: " + std::to_string(min_photon_depth_));

        max_photon_depth_ = params.child_int("max_photon_depth");
        if(max_cam_depth_ < min_photon_depth_)
            throw ObjectConstructionException("invalid max_photon_depth value: " + std::to_string(max_photon_depth_));

        photon_cont_prob_ = params.child_real("photon_cont_prob");
        if(photon_cont_prob_ <= 0 || photon_cont_prob_ > 1)
            throw ObjectConstructionException("invalid photon_cont_prob value: " + std::to_string(photon_cont_prob_));

        alpha_ = params.child_real("alpha");
        if(alpha_ <= 0 || alpha_ >= 1)
            throw ObjectConstructionException("invalid alpha value: " + std::to_string(alpha_));

        worker_count_ = params.child_int("worker_count");

        sampler_prototype_ = SamplerFactory.create(params.child_group("sampler"), init_ctx);

        AGZ_HIERARCHY_WRAP("in initializing sppm renderer")
    }

    void render(Scene &scene, ProgressReporter &reporter, Film *film) override
    {
        // prepare pixel params

        Vec2i res = film->resolution();
        PixelImage pixel_param_img(res.y, res.x);
        for(int y = 0; y < res.y; ++y)
        {
            for(int x = 0; x < res.x; ++x)
                pixel_param_img(y, x).R = init_radius_;
        }

        GBuffer gbuffer(res.y, res.x);

        // pixel image tiling

        std::vector<Grid> grids;
        Grid whole_grid{ 0, res.x, 0, res.y };
        GridDivider::divide(whole_grid, 32, 32, std::back_inserter(grids));

        // perform pm iterations

        scene.start_rendering();
        reporter.begin();
        reporter.message("iterating");
        reporter.new_stage();

        real inv_it_cnt = real(1) / iteration_count_;
        for(int it_idx = 0; it_idx < iteration_count_; ++it_idx)
        {
            perform_iteration(it_idx, scene, grids, pixel_param_img, gbuffer);
            reporter.progress(static_cast<real>(it_idx + 1) * inv_it_cnt * 100);
        }

        reporter.end_stage();

        reporter.message("gathering");
        reporter.new_stage();

        // pixel_param_img -> film

        auto film_grid = film->new_grid(0, res.x, 0, res.y);

        int Ne = iteration_count_ * iteration_photon_count_;
        real factor = 1 / (Ne * PI_r), inv_spp = inv_it_cnt;
        for(int y = 0; y < pixel_param_img.height(); ++y)
        {
            for(int x = 0; x < pixel_param_img.width(); ++x)
            {
                auto &pixel = pixel_param_img(y, x);
                Spectrum L = (pixel.tau * factor / (pixel.R * pixel.R)).clamp_low(0) + pixel.direct_illu * inv_spp;

                GBufferPixel gpixel;
                gpixel.albedo   = inv_spp * gbuffer.albedo  ->at(y, x);
                gpixel.position = inv_spp * gbuffer.position->at(y, x);
                gpixel.normal   = inv_spp * gbuffer.normal  ->at(y, x);
                gpixel.depth    = inv_spp * gbuffer.depth   ->at(y, x);

                film_grid->add_sample({ x + real(0.5), y + real(0.5) }, L, gpixel);
            }
        }

        film->merge_grid(std::move(*film_grid));

        reporter.end_stage();
        reporter.end();
        reporter.message("Total time: " + std::to_string(reporter.total_seconds()) + "s");
    }
};

} // namespace sppm

using SPPMRenderer = sppm::SPPMRenderer;

AGZT_IMPLEMENTATION(Renderer, SPPMRenderer, "sppm")

AGZ_TRACER_END
