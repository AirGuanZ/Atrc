#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/reporter.h>
#include <agz/tracer/core/sampler.h>
#include <agz/tracer/core/scene.h>
#include <agz/utility/thread.h>

#include "perpixel_renderer.h"

AGZ_TRACER_BEGIN

void PerPixelRenderer::render_grid(const Scene &scene, Sampler &sampler, Grid &grid, const Vec2i &full_res) const
{
    Arena arena;
    const Camera *camera = scene.get_camera();
    auto sam_bound = grid.sample_pixels();

    for(int py = sam_bound.low.y; py <= sam_bound.high.y; ++py)
    {
        for(int px = sam_bound.low.x; px <= sam_bound.high.x; ++px)
        {
            sampler.start_pixel(px, py);
            do
            {
                const Sample2 film_sam = sampler.sample2();
                const real pixel_x = px + film_sam.u;
                const real pixel_y = py + film_sam.v;
                const real film_x = pixel_x / full_res.x;
                const real film_y = pixel_y / full_res.y;

                auto cam_ray = camera->sample_we({ film_x, film_y }, sampler.sample2());

                const Ray ray(cam_ray.pos_on_cam, cam_ray.pos_to_out);
                const render::Pixel pixel = eval_pixel(scene, ray, sampler, arena);

                grid.apply(pixel_x, pixel_y, cam_ray.throughput * pixel.value, 1, pixel.albedo, pixel.normal, pixel.denoise);

                arena.release();

                if(stop_rendering_)
                    return;

            } while(sampler.next_sample());
        }
    }
}

template<bool REPORTER_WITH_PREVIEW>
RenderTarget PerPixelRenderer::render_impl(FilmFilterApplier filter, Scene &scene, ProgressReporter &reporter)
{
    int width = filter.width();
    int height = filter.height();
    int x_task_count = (width + task_grid_size_ - 1) / task_grid_size_;
    int y_task_count = (height + task_grid_size_ - 1) / task_grid_size_;
    int total_task_count = x_task_count * y_task_count;

    std::atomic<int> next_task_id = 0;
    std::mutex reporter_mutex;

    ImageBuffer image_buffer(width, height);

    auto get_img = std::function<Image2D<Spectrum>()>([&]()
    {
        auto ratio = image_buffer.weight.map([](real w)
        {
            return w > 0 ? 1 / w : real(1);
        });
        return image_buffer.value * ratio;
    });

    auto func = [
        &filter,
        &scene,
        &reporter,
        &reporter_mutex,
        &next_task_id,
        &get_img,
        x_task_count,
        total_task_count,
        task_grid_size = task_grid_size_,
        this
    ] (Sampler *sampler, ImageBuffer *image_buffer)
    {
        const Vec2i full_res = { filter.width(), filter.height() };

        for(;;)
        {
            if(stop_rendering_)
                return;

            const int task_id = next_task_id++;
            if(task_id >= total_task_count)
                break;

            const int grid_x_index = task_id % x_task_count;
            const int grid_y_index = task_id / x_task_count;
            const int x_beg = grid_x_index * task_grid_size;
            const int y_beg = grid_y_index * task_grid_size;
            const int x_end = (std::min)(x_beg + task_grid_size, full_res.x);
            const int y_end = (std::min)(y_beg + task_grid_size, full_res.y);

            auto grid = filter.create_subgrid<Spectrum, real, Spectrum, Vec3, real>(
                { { x_beg, y_beg }, { x_end - 1, y_end - 1 } });
            this->render_grid(scene, *sampler, grid, full_res);

            if constexpr(REPORTER_WITH_PREVIEW)
            {
                const real percent = real(100) * (task_id + 1) / total_task_count;
                std::lock_guard lk(reporter_mutex);
                grid.merge_into(
                    image_buffer->value, image_buffer->weight,
                    image_buffer->albedo, image_buffer->normal, image_buffer->denoise);
                reporter.progress(percent, get_img);
            }
            else
            {
                AGZ_UNACCESSED(get_img);

                grid.merge_into(
                    image_buffer->value, image_buffer->weight,
                    image_buffer->albedo, image_buffer->normal, image_buffer->denoise);

                const real percent = real(100) * (task_id + 1) / total_task_count;
                std::lock_guard lk(reporter_mutex);
                reporter.progress(percent, {});
            }
        }
    };

    const int worker_count = thread::actual_worker_count(worker_count_);
    std::vector<std::thread> threads;

    Arena sampler_arena;

    reporter.begin();
    reporter.new_stage();

    for(int i = 0; i < worker_count; ++i)
    {
        auto sampler = sampler_prototype_->clone(i, sampler_arena);
        threads.emplace_back(func, sampler, &image_buffer);
    }

    for(auto &t : threads)
        t.join();

    reporter.end_stage();
    reporter.end();

    auto ratio = image_buffer.weight.map([](real w)
    {
        return w > 0 ? 1 / w : real(1);
    });

    RenderTarget render_target;
    render_target.image = image_buffer.value * ratio;
    render_target.albedo = image_buffer.albedo * ratio;
    render_target.normal = image_buffer.normal * ratio;
    render_target.denoise = image_buffer.denoise * ratio;

    reporter.message("total time: " + std::to_string(reporter.total_seconds()) + "s");

    return render_target;
}

PerPixelRenderer::PerPixelRenderer(int worker_count, int task_grid_size, std::shared_ptr<const Sampler> sampler_prototype)
    : worker_count_(worker_count), task_grid_size_(task_grid_size), sampler_prototype_(std::move(sampler_prototype))
{
    
}

RenderTarget PerPixelRenderer::render(FilmFilterApplier filter, Scene &scene, ProgressReporter &reporter)
{
    if(reporter.need_image_preview())
        return render_impl<true>(filter, scene, reporter);
    return render_impl<false>(filter, scene, reporter);
}

AGZ_TRACER_END
