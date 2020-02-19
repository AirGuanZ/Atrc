#include <agz/editor/renderer/renderer.h>
#include <agz/utility/thread.h>

AGZ_EDITOR_BEGIN

PerPixelRenderer::PerPixelRenderer(
    int worker_count, int task_grid_size, int init_pixel_size,
    int framebuffer_width, int framebuffer_height,
    bool enable_fast_rendering, int fast_resolution, int fast_task_grid_size,
    std::shared_ptr<const tracer::Scene> scene)
    : framebuffer_(framebuffer_width, framebuffer_height, task_grid_size, init_pixel_size)
{
    worker_count_          = worker_count;
    framebuffer_width_     = framebuffer_width;
    framebuffer_height_    = framebuffer_height;
    enable_fast_rendering_ = enable_fast_rendering;
    fast_resolution_       = fast_resolution;
    fast_task_grid_size_   = fast_task_grid_size;
    scene_                 = std::move(scene);
    stop_rendering_        = false;
}

PerPixelRenderer::~PerPixelRenderer()
{
    assert(threads_.empty());
}

Image2D<math::color3b> PerPixelRenderer::start()
{
    auto render_func = [this](tracer::Sampler *sampler)
    {
        std::vector<Framebuffer::Task> tasks;

        for(;;)
        {
            if(stop_rendering_)
                return;

            tasks.clear();
            int task_count = framebuffer_.get_tasks(2, tasks);

            for(int i = 0; i < task_count; ++i)
                exec_render_task(tasks[i], sampler);

            framebuffer_.merge_tasks(task_count, tasks.data());
        }
    };

    auto ret = do_fast_rendering();

    auto sampler_prototype = tracer::create_native_sampler(1, 0, true);
    int worker_count = thread::actual_worker_count(worker_count_);
    for(int i = 0; i < worker_count; ++i)
    {
        auto sampler = sampler_prototype->clone(i, sampler_arena_);
        threads_.emplace_back(render_func, sampler);
    }

    framebuffer_.start();

    return ret;
}

Image2D<math::color3b> PerPixelRenderer::get_image() const
{
    return framebuffer_.get_image();
}

void PerPixelRenderer::stop_rendering()
{
    stop_rendering_ = true;
    for(auto &t : threads_)
    {
        if(t.joinable())
            t.join();
    }
    threads_.clear();
}

Image2D<math::color3b> PerPixelRenderer::do_fast_rendering()
{
    using namespace tracer;

    if(!enable_fast_rendering_)
        return {};

    const real target_ratio = static_cast<real>(framebuffer_width_) / framebuffer_height_;
    
    int small_width, small_height;
    if(target_ratio < 1)
    {
        small_width = (std::max)(1, static_cast<int>(std::floor(fast_resolution_ * target_ratio)));
        small_height = fast_resolution_;
    }
    else
    {
        small_width = fast_resolution_;
        small_height = (std::max)(1, static_cast<int>(std::floor(fast_resolution_ / target_ratio)));
    }

    Image2D<math::color3b> small_target(small_height, small_width);

    const int x_task_count = (small_width + fast_task_grid_size_ - 1) / fast_task_grid_size_;
    const int y_task_count = (small_height + fast_task_grid_size_ - 1) / fast_task_grid_size_;
    const int total_task_count = x_task_count * y_task_count;
    std::atomic<int> next_task_id = 0;

    const auto render_func = [
        this,
        &scene = *scene_,
        &small_target,
        total_task_count,
        x_task_count,
        &next_task_id
    ]
    (Sampler *sampler)
    {
        for(;;)
        {
            int task_id = next_task_id++;
            if(task_id >= total_task_count)
                return;

            const int x_task_id = task_id % x_task_count;
            const int y_task_id = task_id / x_task_count;

            const int x_beg = x_task_id * fast_task_grid_size_;
            const int y_beg = y_task_id * fast_task_grid_size_;

            const int x_end = (std::min)(x_beg + fast_task_grid_size_, small_target.width());
            const int y_end = (std::min)(y_beg + fast_task_grid_size_, small_target.height());

            exec_fast_render_task(small_target, { x_beg, y_beg }, { x_end, y_end }, *sampler);
        }
    };

    const int worker_count = thread::actual_worker_count(worker_count_);
    auto sampler_prototype = create_native_sampler(1, 42, true);

    Arena sampler_arena;
    std::vector<std::thread> threads;
    for(int i = 0; i < worker_count; ++i)
    {
        Sampler *sampler = sampler_prototype->clone(i, sampler_arena);
        threads.emplace_back(render_func, sampler);
    }

    for(auto &t : threads)
        t.join();

    return small_target;
}

void PerPixelRenderer::exec_fast_render_task(Image2D<math::color3b> &target, const Vec2i &beg, const Vec2i &end, tracer::Sampler &sampler)
{
    using namespace tracer;

    Arena arena;

    const Camera *camera = scene_->get_camera();

    for(int py = beg.y; py < end.y; ++py)
    {
        for(int px = beg.x; px < end.x; ++px)
        {
            sampler.start_pixel(px, py);

            Spectrum sum;
            do
            {
                const Sample2 film_sam = sampler.sample2();
                const real pixel_x = px + film_sam.u;
                const real pixel_y = py + film_sam.v;
                const real film_x = pixel_x / target.width();
                const real film_y = pixel_y / target.height();

                const auto cam_ray = camera->sample_we({ film_x, film_y }, sampler.sample2());

                const Ray ray(cam_ray.pos_on_cam, cam_ray.pos_to_out);
                const Spectrum radiance = cam_ray.throughput * fast_render_pixel(*scene_, ray, sampler, arena);

                sum += radiance;

                if(arena.used_bytes() > 4 * 1024 * 1024)
                    arena.release();

            } while(sampler.next_sample());

            const Spectrum rad = sum / real(sampler.get_sample_count());
            target(py, px) = to_color3b(rad.map([](real c) { return std::pow(c, real(1 / 2.2)); }));
        }
    }
}

void PerPixelRenderer::exec_render_task(Framebuffer::Task &task, tracer::Sampler *sampler)
{
    using namespace tracer;

    Arena arena;
    const Camera *camera = scene_->get_camera();
    const Rect2i sam_bound = task.pixel_range;

    task.grid->value.clear(Spectrum());
    task.grid->weight.clear(0);

    for(int py = sam_bound.low.y; py <= sam_bound.high.y; ++py)
    {
        for(int px = sam_bound.low.x; px <= sam_bound.high.x; ++px)
        {
            sampler->start_pixel(px, py);

            for(int s = 0; s < task.spp; ++s)
            {
                if(stop_rendering_)
                    return;

                const Sample2 film_sam = sampler->sample2();
                const real pixel_x = px + film_sam.u;
                const real pixel_y = py + film_sam.v;
                const real film_x = pixel_x / task.full_res.x;
                const real film_y = pixel_y / task.full_res.y;

                auto cam_ray = camera->sample_we({ film_x, film_y }, sampler->sample2());

                const Ray ray(cam_ray.pos_on_cam, cam_ray.pos_to_out);
                const Spectrum radiance = cam_ray.throughput * this->render_pixel(*scene_, ray, *sampler, arena);

                const int lx = px - sam_bound.low.x;
                const int ly = py - sam_bound.low.y;

                if(radiance.is_finite())
                {
                    task.grid->value(ly, lx) += radiance;
                    task.grid->weight(ly, lx) += 1;
                }
                else
                {
                    task.grid->value(ly, lx) += Spectrum(real(0.001));
                    task.grid->weight(ly, lx) += real(0.001);
                }

                if(arena.used_bytes() > 4 * 1024 * 1024)
                    arena.release();
            }
        }
    }
}

AGZ_EDITOR_END
