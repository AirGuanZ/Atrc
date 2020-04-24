#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/renderer_interactor.h>
#include <agz/tracer/core/sampler.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/create/renderer.h>
#include <agz/tracer/render/photon_mapping.h>
#include <agz/tracer/utility/parallel_grid.h>
#include <agz/utility/thread.h>

AGZ_TRACER_BEGIN

class SPPMRenderer : public Renderer
{
public:

    explicit SPPMRenderer(const SPPMRendererParams &params);

    RenderTarget render(
        FilmFilterApplier filter, Scene &scene,
        RendererInteractor &reporter) override;

private:

    SPPMRendererParams params_;
};

SPPMRenderer::SPPMRenderer(const SPPMRendererParams &params)
    : params_(params)
{

}

RenderTarget SPPMRenderer::render(
    FilmFilterApplier filter, Scene &scene, RendererInteractor &reporter)
{
    const int thread_count = thread::actual_worker_count(params_.worker_count);

    std::mutex reporter_mutex;

    reporter.begin();
    reporter.new_stage();

    // determine initial search radius

    AABB world_bound = scene.world_bound();

    real init_radius = params_.init_radius;
    if(init_radius < 0)
        init_radius = (world_bound.high - world_bound.low).length() / 1000;

    world_bound.low  -= Vec3(init_radius);
    world_bound.high += Vec3(init_radius);

    // initialize pixels

    Image2D<Spectrum> albedo_buffer (filter.height(), filter.width());
    Image2D<Vec3>     normal_buffer (filter.height(), filter.width());
    Image2D<real>     denoise_buffer(filter.height(), filter.width());

    Image2D<render::sppm::Pixel> sppm_pixels(filter.height(), filter.width());
    for(int y = 0; y < filter.height(); ++y)
    {
        for(int x = 0; x < filter.width(); ++x)
            sppm_pixels(y, x).radius = init_radius;
    }

    // samplers

    Arena sampler_arena;
    auto sampler_prototype = newRC<NativeSampler>(42, false);

    std::vector<Sampler *> perthread_sampler;
    for(int i = 0; i < thread_count; ++i)
        perthread_sampler.push_back(sampler_prototype->clone(i, sampler_arena));

    // vp arenas

    std::vector<Arena> perthread_vp_arena(thread_count);

    // how to compute the final image

    auto compute_image = [&](int iter_cnt, uint64_t photon_cnt)
    {
        Image2D<Spectrum> img(filter.height(), filter.width());

        for(int y = 0; y < filter.height(); ++y)
        {
            for(int x = 0; x < filter.width(); ++x)
            {
                img(y, x) = compute_pixel_radiance(
                    iter_cnt, photon_cnt, sppm_pixels(y, x));
            }
        }

        return img;
    };

    // run sppm iterations

    real max_radius = init_radius;

    thread::thread_group_t thread_group;

    for(int iter = 0; iter < params_.iteration_count; ++iter)
    {
        if(stop_rendering_)
            return {};

        if(reporter.need_image_preview())
            reporter.message("start iter " + std::to_string(iter + 1));

        const real progress_beg = 100 * real(iter)     / params_.iteration_count;
        const real progress_end = 100 * real(iter + 1) / params_.iteration_count;
        const real progress_mid = (progress_beg + progress_end) / 2;

        reporter.progress(progress_beg, {});

        // clear visible points

        const real grid_sidelen = real(1.05) * max_radius;
        render::sppm::VisiblePointSearcher vp_searcher(
            world_bound, grid_sidelen, 40960);

        for(auto &a : perthread_vp_arena)
            a.release();

        // find new visible points

        int finished_pixel_count = 0;

        parallel_for_2d_grid(
            thread_count, filter.width(), filter.height(),
            params_.forward_task_grid_size, params_.forward_task_grid_size,
            thread_group,
            [&](int thread_index, const Rect2i &grid)
        {
            auto camera    = scene.get_camera();
            auto sampler   = perthread_sampler [thread_index];
            auto &vp_arena = perthread_vp_arena[thread_index];

            for(int y = grid.low.y; y < grid.high.y; ++y)
            {
                for(int x = grid.low.x; x < grid.high.x; ++x)
                {
                    const Sample2 film_sam = sampler->sample2();
                    const Vec2 film_coord = {
                        (x + film_sam.u) / filter.width(),
                        (y + film_sam.v) / filter.height()
                    };

                    const CameraSampleWeResult cam_sam = camera->sample_we(
                        film_coord, sampler->sample2());

                    const Ray ray(cam_sam.pos_on_cam, cam_sam.pos_to_out);

                    render::GBufferPixel gpixel;

                    auto &pixel = sppm_pixels(y, x);
                    pixel.vp = render::sppm::tracer_vp(
                        params_.forward_max_depth, 1,
                        scene, ray, cam_sam.throughput,
                        vp_arena, *sampler, &gpixel, pixel.direct_illum);

                    if(pixel.vp.is_valid())
                        vp_searcher.add_vp(pixel, vp_arena);

                    albedo_buffer(y, x) += gpixel.albedo;
                    normal_buffer(y, x) += gpixel.normal;
                    denoise_buffer(y, x) += gpixel.denoise;
                }

                if(stop_rendering_)
                    return false;
            }

            std::lock_guard lk(reporter_mutex);
            finished_pixel_count += (grid.high - grid.low).product();

            const real t = real(finished_pixel_count)
                         / (filter.width() * filter.height());
            reporter.progress(
                math::lerp(progress_beg, progress_mid, t), {});

            return true;
        });

        reporter.progress(progress_mid, {});

        // trace photons

        int finished_photon_count = 0;
        parallel_for_1d_grid(
            thread_count,
            params_.photons_per_iteration,
            4096,
            thread_group,
            [&](int thread_index, int beg, int end)
        {
            auto sampler = perthread_sampler[thread_index];
            Arena local_arena;
            for(int i = beg; i < end; ++i)
            {
                trace_photon(
                    params_.photon_min_depth,
                    params_.photon_max_depth,
                    params_.photon_cont_prob,
                    vp_searcher, scene, local_arena, *sampler);

                if(local_arena.used_bytes() > 4 * 1024 * 1024)
                    local_arena.release();

                if(stop_rendering_)
                    return false;
            }

            std::lock_guard lk(reporter_mutex);
            finished_photon_count += end - beg;

            const real t = real(finished_photon_count)
                         / params_.photons_per_iteration;
            reporter.progress(
                math::lerp(progress_mid, progress_end, t), {});

            return true;
        });

        // update pixel params

        max_radius = 0;
        for(int y = 0; y < filter.height(); ++y)
        {
            for(int x = 0; x < filter.width(); ++x)
            {
                if(sppm_pixels(y, x).vp.is_valid())
                {
                    max_radius = (std::max)(max_radius,
                                            sppm_pixels(y, x).radius);
                }
            }
        }

        if(max_radius == 0)
            max_radius = init_radius;

        parallel_for_1d_grid(
            thread_count, filter.height(), 128, thread_group,
            [&](int thread_index, int beg, int end)
        {
            for(int y = beg; y < end; ++y)
            {
                for(int x = 0; x < filter.width(); ++x)
                {
                    if(sppm_pixels(y, x).vp.is_valid())
                    {
                        update_pixel_params(
                            params_.update_alpha, sppm_pixels(y, x));
                    }
                }
            }
        });

        // report progress

        if(reporter.need_image_preview())
        {
            reporter.progress(
                progress_end, [&compute_image, iter, this]
            {
                int iter_cnt = iter + 1;
                return compute_image(
                    iter_cnt,
                    uint64_t(iter_cnt) * params_.photons_per_iteration);
            });
        }
        else
            reporter.progress(progress_end, {});
    }

    reporter.end_stage();
    reporter.end();

    // compute final image

    RenderTarget ret;

    const uint64_t photon_count = uint64_t(params_.iteration_count)
                                * uint64_t(params_.photons_per_iteration);
    ret.image = compute_image(params_.iteration_count, photon_count);

    const real gbuffer_ratio = 1 / real(params_.iteration_count);
    ret.albedo  = albedo_buffer  * gbuffer_ratio;
    ret.normal  = normal_buffer  * gbuffer_ratio;
    ret.denoise = denoise_buffer * gbuffer_ratio;

    return ret;
}

RC<Renderer> create_sppm_renderer(const SPPMRendererParams &params)
{
    return newRC<SPPMRenderer>(params);
}

AGZ_TRACER_END
