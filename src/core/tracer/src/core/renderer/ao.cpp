#include <atomic>
#include <mutex>
#include <thread>

#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/renderer.h>
#include <agz/tracer/core/reporter.h>
#include <agz/tracer/core/sampler.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/factory/raw/renderer.h>
#include <agz/utility/thread.h>

AGZ_TRACER_BEGIN

class AORenderer : public Renderer
{
    using ImageBuffer = ImageBufferTemplate<true, true, true, true, true>;

    struct Pixel
    {
        Spectrum value;
        Spectrum albedo;
        Vec3     normal;
        real     denoise = 1;
    };

    // image value, weight, albedo, normal, d
    using Grid = FilmFilterApplier::FilmGrid<Spectrum, real, Spectrum, Vec3, real>;

    AORendererParams params_;

    Pixel trace_camera_ray(const Scene &scene, const Ray &r, Sampler &sampler) const
    {
        EntityIntersection inct;
        if(!scene.closest_intersection(r, &inct))
            return { params_.background_color, {}, {}, 1 };

        Spectrum pixel_albedo  = params_.high_color;
        Vec3     pixel_normal  = inct.geometry_coord.z;
        real     pixel_denoise = inct.entity->get_no_denoise_flag() ? real(0) : real(1);;
        
        const Vec3 start_pos = inct.eps_offset(inct.geometry_coord.z);

        real ao_factor = 0;
        for(int i = 0; i < params_.ao_sample_count; ++i)
        {
            const Sample2 sam = sampler.sample2();
            const Vec3 local_dir = math::distribution::zweighted_on_hemisphere(sam.u, sam.v).first;
            const Vec3 global_dir = inct.geometry_coord.local_to_global(local_dir).normalize();

            const Vec3 end_pos = start_pos + global_dir * params_.max_occlusion_distance;
            if(scene.visible(start_pos, end_pos))
                ao_factor += 1;
        }
        ao_factor /= params_.ao_sample_count;

        return {
            lerp(params_.low_color, params_.high_color, math::saturate(ao_factor)),
            pixel_albedo, pixel_normal, pixel_denoise
        };
    }

    void render_grid(const Scene &scene, Sampler &sampler, Grid &grid, const Vec2i &full_res) const
    {
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
                    const Pixel pixel = trace_camera_ray(scene, ray, sampler);
                    
                    grid.apply(pixel_x, pixel_y, cam_ray.throughput * pixel.value, 1, pixel.albedo, pixel.normal, pixel.denoise);

                } while(sampler.next_sample());
            }
        }
    }

public:

    explicit AORenderer(const AORendererParams &params)
        : params_(params)
    {

    }

    RenderTarget render(const FilmFilterApplier &filter, Scene &scene, ProgressReporter &reporter) override
    {
        int width  = filter.width();
        int height = filter.height();
        int x_task_count = (width + params_.task_grid_size - 1) / params_.task_grid_size;
        int y_task_count = (height + params_.task_grid_size - 1) / params_.task_grid_size;
        int total_task_count = x_task_count * y_task_count;

        std::atomic<int> next_task_id = 0;
        std::mutex reporter_mutex;

        auto func = [
            &filter,
            &scene,
            &reporter,
            &reporter_mutex,
            &next_task_id,
            x_task_count,
            total_task_count,
            task_grid_size = params_.task_grid_size,
            this
        ] (Sampler *sampler, ImageBuffer *image_buffer)
        {
            const Vec2i full_res = { filter.width(), filter.height() };

            for(;;)
            {
                const int task_id = next_task_id++;
                if(task_id >= total_task_count)
                    break;

                const int grid_x_index = task_id % x_task_count;
                const int grid_y_index = task_id / x_task_count;
                const int x_beg = grid_x_index * task_grid_size;
                const int y_beg = grid_y_index * task_grid_size;
                const int x_end = (std::min)(x_beg + task_grid_size, full_res.x);
                const int y_end = (std::min)(y_beg + task_grid_size, full_res.y);

                auto grid = filter.bind_to(
                    { { x_beg, y_beg }, { x_end - 1, y_end - 1 } },
                    image_buffer->value, image_buffer->weight,
                    image_buffer->albedo, image_buffer->normal, image_buffer->denoise);
                this->render_grid(scene, *sampler, grid, full_res);

                const real percent = real(100) * (task_id + 1) / total_task_count;
                std::lock_guard lk(reporter_mutex);
                reporter.progress(percent);
            }
        };

        const int worker_count = thread::actual_worker_count(params_.worker_count);
        std::vector<std::thread> threads;

        Arena sampler_arena;

        ImageBuffer image_buffer(width, height);

        scene.start_rendering();

        reporter.begin();
        reporter.new_stage();

        for(int i = 0; i < worker_count; ++i)
        {
            auto sampler = params_.sampler_prototype->clone(i, sampler_arena);
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
        render_target.image   = image_buffer.value * ratio;
        render_target.albedo  = image_buffer.albedo * ratio;
        render_target.normal  = image_buffer.normal * ratio;
        render_target.denoise = image_buffer.denoise * ratio;

        reporter.message("total time: " + std::to_string(reporter.total_seconds()) + "s");

        return render_target;
    }
};

std::shared_ptr<Renderer> create_ao_renderer(const AORendererParams &params)
{
    return std::make_shared<AORenderer>(params);
}

AGZ_TRACER_END
