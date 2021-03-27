#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/renderer_interactor.h>
#include <agz/tracer/core/sampler.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/utility/parallel_grid.h>
#include <agz/tracer/utility/perthread_samplers.h>
#include <agz-utils/thread.h>

#include "perpixel_renderer.h"

AGZ_TRACER_BEGIN

void PerPixelRenderer::render_grid(
    const Scene &scene, Sampler &sampler,
    Grid &grid, const Vec2i &full_res, int spp) const
{
    Arena arena;
    const Camera *camera = scene.get_camera();
    auto sam_bound = grid.sample_pixels();

    for(int py = sam_bound.low.y; py <= sam_bound.high.y; ++py)
    {
        for(int px = sam_bound.low.x; px <= sam_bound.high.x; ++px)
        {
            for(int i = 0; i < spp; ++i)
            {
                const Sample2 film_sam = sampler.sample2();
                const real pixel_x = px + film_sam.u;
                const real pixel_y = py + film_sam.v;
                const real film_x = pixel_x / full_res.x;
                const real film_y = pixel_y / full_res.y;

                auto cam_ray = camera->sample_we(
                    { film_x, film_y }, sampler.sample2());

                const Ray ray(cam_ray.pos_on_cam, cam_ray.pos_to_out);
                const render::Pixel pixel = eval_pixel(
                    scene, ray, sampler, arena);

                if(pixel.value.is_finite())
                {
                    grid.apply(
                        pixel_x, pixel_y, cam_ray.throughput * pixel.value, 1,
                        pixel.albedo, pixel.normal, pixel.denoise);
                }

                arena.release();

                if(stop_rendering_)
                    return;
            }
        }
    }
}

template<bool REPORTER_WITH_PREVIEW>
RenderTarget PerPixelRenderer::render_impl(
    FilmFilterApplier filter, Scene &scene, RendererInteractor &reporter)
{
    const int thread_count = thread::actual_worker_count(worker_count_);

    // prepare image buffer

    ImageBuffer image_buffer(filter.width(), filter.height());

    auto get_img = std::function<Image2D<Spectrum>()>([&]()
    {
        auto ratio = image_buffer.weight.map([](real w)
        {
            return w > 0 ? 1 / w : real(1);
        });
        return image_buffer.value * ratio;
    });

    // create per-thread samplers

    auto sampler_prototype = newRC<NativeSampler>(42, false);

    PerThreadNativeSamplers perthread_sampler(
        thread_count, *sampler_prototype);

    std::mutex reporter_mutex;

    reporter.begin();
    reporter.new_stage();

    // rendering iteration

    thread::thread_group_t thread_group(thread_count);

    auto run_iter = [&](double prog_beg, double prog_end, int spp)
    {
        int finished_pixel_count = 0;

        parallel_for_2d_grid(
            thread_count, filter.width(), filter.height(),
            task_grid_size_, task_grid_size_, thread_group,
            [&] (int thread_index, const Rect2i &rect)
        {
            auto sampler = perthread_sampler.get_sampler(thread_index);

            auto grid = filter.create_subgrid<
                Spectrum, real, Spectrum, Vec3, real>(
                    { rect.low, rect.high - Vec2i(1) });

            render_grid(
                scene, *sampler, grid,
                { filter.width(), filter.height() }, spp);

            const int total_pixel_count = filter.width() * filter.height();

            if constexpr(REPORTER_WITH_PREVIEW)
            {
                std::lock_guard lk(reporter_mutex);
                grid.merge_into(
                    image_buffer.value, image_buffer.weight,
                    image_buffer.albedo, image_buffer.normal,
                    image_buffer.denoise);

                finished_pixel_count += (rect.high - rect.low).product();
                const double percent = math::lerp(
                    prog_beg, prog_end,
                    double(finished_pixel_count) / total_pixel_count);
                reporter.progress(percent, get_img);
            }
            else
            {
                AGZ_UNACCESSED(get_img);

                grid.merge_into(
                    image_buffer.value, image_buffer.weight,
                    image_buffer.albedo, image_buffer.normal,
                    image_buffer.denoise);

                std::lock_guard lk(reporter_mutex);

                finished_pixel_count += (rect.high - rect.low).product();
                const double percent = math::lerp(
                    prog_beg, prog_end,
                    double(finished_pixel_count) / total_pixel_count);
                reporter.progress(percent, {});
            }

            return !stop_rendering_;
        });
    };

    // start rendering

    if(reporter.need_image_preview())
    {
        const double first_iter_prog_end = 100.0 / spp_;
        run_iter(0, first_iter_prog_end, 1);

        const int per_iter_spp = (std::max)(6, spp_ / 20);
        int finished_spp = 1;
        while(finished_spp < spp_)
        {
            if(stop_rendering_)
                break;

            const int new_finished_spp = (std::min)(
                spp_, finished_spp + per_iter_spp);
            const int delta_spp = new_finished_spp - finished_spp;

            const double prog_beg = 100.0 * finished_spp / spp_;
            const double prog_end = 100.0 * new_finished_spp / spp_;

            run_iter(prog_beg, prog_end, delta_spp);

            finished_spp = new_finished_spp;
        }
    }
    else
        run_iter(0, 100, spp_);

    reporter.end_stage();
    reporter.end();

    auto ratio = image_buffer.weight.map([](real w)
    {
        return w > 0 ? 1 / w : real(1);
    });

    RenderTarget render_target;
    render_target.image   = image_buffer.value   * ratio;
    render_target.albedo  = image_buffer.albedo  * ratio;
    render_target.normal  = image_buffer.normal  * ratio;
    render_target.denoise = image_buffer.denoise * ratio;

    return render_target;
}

PerPixelRenderer::PerPixelRenderer(
    int worker_count, int task_grid_size, int spp)
    : worker_count_(worker_count), task_grid_size_(task_grid_size), spp_(spp)
{
    
}

RenderTarget PerPixelRenderer::render(
    FilmFilterApplier filter, Scene &scene, RendererInteractor &reporter)
{
    if(reporter.need_image_preview())
        return render_impl<true>(filter, scene, reporter);
    return render_impl<false>(filter, scene, reporter);
}

AGZ_TRACER_END
