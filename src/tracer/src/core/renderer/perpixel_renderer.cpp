#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/renderer_interactor.h>
#include <agz/tracer/core/sampler.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/utility/parallel_grid.h>
#include <agz/utility/thread.h>

#include "perpixel_renderer.h"

AGZ_TRACER_BEGIN

void PerPixelRenderer::render_grid(
    const Scene &scene, Sampler &sampler,
    Grid &grid, const Vec2i &full_res) const
{
    Arena arena;
    const Camera *camera = scene.get_camera();
    auto sam_bound = grid.sample_pixels();

    for(int py = sam_bound.low.y; py <= sam_bound.high.y; ++py)
    {
        for(int px = sam_bound.low.x; px <= sam_bound.high.x; ++px)
        {
            for(int i = 0; i < spp_; ++i)
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

    Arena sampler_arena;
    auto sampler_prototype = newRC<NativeSampler>(42, false);
    std::vector<Sampler *> perthread_sampler;
    for(int i = 0; i < thread_count; ++i)
        perthread_sampler.push_back(sampler_prototype->clone(i, sampler_arena));

    int finished_pixel_count = 0;
    std::mutex reporter_mutex;

    reporter.begin();
    reporter.new_stage();

    // start rendering

    parallel_for_2d_grid(
        thread_count, filter.width(), filter.height(),
        task_grid_size_, task_grid_size_,
        [
            &filter,
            &scene,
            &reporter,
            &reporter_mutex,
            &image_buffer,
            &finished_pixel_count,
            &get_img,
            &perthread_sampler,
            this
        ] (int thread_index, const Rect2i &rect)
    {
        auto &sampler = perthread_sampler[thread_index];

        auto grid = filter.create_subgrid<
            Spectrum, real, Spectrum, Vec3, real>(
                { rect.low, rect.high - Vec2i(1) });

        render_grid(scene, *sampler, grid, { filter.width(), filter.height() });

        if constexpr(REPORTER_WITH_PREVIEW)
        {
            std::lock_guard lk(reporter_mutex);
            grid.merge_into(
                image_buffer.value, image_buffer.weight,
                image_buffer.albedo, image_buffer.normal,
                image_buffer.denoise);

            finished_pixel_count += (rect.high - rect.low).product();
            const real percent = real(100) * finished_pixel_count
                               / (filter.width() * filter.height());
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
            const real percent = real(100) * finished_pixel_count
                               / (filter.width() * filter.height());
            reporter.progress(percent, {});
        }

        return !stop_rendering_;
    });

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
