#include <agz/editor/renderer/particle_renderer.h>
#include <agz/utility/thread.h>

AGZ_EDITOR_BEGIN

ParticleRenderer::ParticleRenderer(
    int worker_count, int task_grid_size, int init_pixel_size,
    int framebuffer_width, int framebuffer_height,
    bool enable_fast_rendering, int fast_resolution, int fast_task_size,
    RC<const tracer::Scene> scene)
    : framebuffer_(
        framebuffer_width, framebuffer_height, task_grid_size, init_pixel_size)
{
    worker_count_          = worker_count;
    framebuffer_width_     = framebuffer_width;
    framebuffer_height_    = framebuffer_height;
    enable_fast_rendering_ = enable_fast_rendering;
    fast_resolution_       = fast_resolution;
    fast_task_grid_size_   = fast_task_size;
    scene_                 = std::move(scene);

    stop_rendering_               = false;
    global_particle_film_version_ = 0;
    total_particle_count_         = 0;
}

ParticleRenderer::~ParticleRenderer()
{
    assert(threads_.empty());
}

Image2D<Spectrum> ParticleRenderer::start()
{
    auto render_func = [this](tracer::Sampler *sampler, int thread_idx)
    {
        auto &particle_film_mutex = particle_film_mutex_[thread_idx];
        auto &particle_film       = particle_film_      [thread_idx];

        Image2D<Spectrum> local_particle_film(
            film_filter_->height(), film_filter_->width());
        auto local_particle_film_view = film_filter_->create_subgrid_view(
            { { 0, 0 }, { film_filter_->width() - 1, film_filter_->height() - 1 } },
            local_particle_film);

        int local_particle_film_version = -1;
        uint64_t local_particle_count        = 0;
        
        std::vector<Framebuffer::Task> tasks;

        for(;;)
        {
            if(stop_rendering_)
                return;

            tasks.clear();
            const int task_count = framebuffer_.get_tasks(2, tasks);

            for(int i = 0; i < task_count; ++i)
                local_particle_count += exec_render_task(
                    tasks[i], *sampler, local_particle_film_view);

            framebuffer_.merge_tasks(task_count, tasks.data());

            const int current_version = global_particle_film_version_;
            if(local_particle_film_version != current_version)
            {
                local_particle_film_version = current_version;

                std::lock_guard lk(particle_film_mutex);
                particle_film = local_particle_film;
                total_particle_count_ += local_particle_count;
                local_particle_count = 0;
            }
        }
    };

    const auto ret = do_fast_rendering();

    const auto sampler_prototype = newRC<tracer::Sampler>(0, true);
    const int worker_count = thread::actual_worker_count(worker_count_);

    film_filter_ = newBox<tracer::FilmFilterApplier>(
        framebuffer_width_, framebuffer_height_,
        tracer::create_box_filter(real(0.5)));

    particle_film_.resize(worker_count);
    particle_film_mutex_ = newBox<std::mutex[]>(worker_count);

    for(int i = 0; i < worker_count; ++i)
    {
        auto sampler = sampler_prototype->clone(i, sampler_arena_);
        threads_.emplace_back(render_func, sampler, i);
    }

    framebuffer_.start();

    auto compute_image_func = [=]
    {
        const std::chrono::milliseconds wait_ms(20);

        Image2D<Spectrum> particle_output(
            framebuffer_height_, framebuffer_width_);

        for(;;)
        {
            for(int i = 0; i < 15; ++i)
            {
                if(stop_rendering_)
                    return;
                std::this_thread::sleep_for(wait_ms);
            }

            auto framebuffer_output = framebuffer_.get_image();
            if(!framebuffer_output.is_available())
                continue;

            particle_output.clear(Spectrum());
            for(int i = 0; i < worker_count; ++i)
            {
                if(stop_rendering_)
                    return;

                std::lock_guard lk(particle_film_mutex_[i]);
                if(particle_film_[i].is_available())
                    particle_output += particle_film_[i];
            }

            const real particle_ratio = real(
                framebuffer_width_ * framebuffer_height_) / total_particle_count_;

            for(int y = 0; y < framebuffer_height_; ++y)
            {
                if(stop_rendering_)
                    return;

                for(int x = 0; x < framebuffer_width_; ++x)
                    framebuffer_output(y, x) += particle_ratio
                                              * particle_output(y, x);
            }

            ++global_particle_film_version_;

            std::lock_guard lk(output_img_mutex_);
            output_img_.swap(framebuffer_output);
        }
    };

    compute_img_thread_ = std::thread(compute_image_func);

    return ret;
}

Image2D<Spectrum> ParticleRenderer::get_image() const
{
    std::lock_guard lk(output_img_mutex_);
    return output_img_;
}

void ParticleRenderer::stop_rendering()
{
    stop_rendering_ = true;
    for(auto &t : threads_)
    {
        if(t.joinable())
            t.join();
    }
    compute_img_thread_.join();
    threads_.clear();
}

Image2D<Spectrum> ParticleRenderer::do_fast_rendering()
{
    using namespace tracer;

    if(!enable_fast_rendering_)
        return {};

    const real target_ratio = static_cast<real>(framebuffer_width_)
                            / framebuffer_height_;

    int small_width, small_height;
    if(target_ratio < 1)
    {
        small_width = (std::max)(
            1, static_cast<int>(std::floor(fast_resolution_ * target_ratio)));
        small_height = fast_resolution_;
    }
    else
    {
        small_width = fast_resolution_;
        small_height = (std::max)(
            1, static_cast<int>(std::floor(fast_resolution_ / target_ratio)));
    }

    Image2D<Spectrum> small_target(small_height, small_width);

    const int x_task_count = (small_width + fast_task_grid_size_ - 1)
                           / fast_task_grid_size_;
    const int y_task_count = (small_height + fast_task_grid_size_ - 1)
                           / fast_task_grid_size_;
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

            const int x_end = (std::min)(
                x_beg + fast_task_grid_size_, small_target.width());
            const int y_end = (std::min)(
                y_beg + fast_task_grid_size_, small_target.height());

            exec_fast_render_task(
                small_target, { x_beg, y_beg }, { x_end, y_end }, *sampler);
        }
    };

    const int worker_count = thread::actual_worker_count(worker_count_);
    auto sampler_prototype = newRC<tracer::Sampler>(42, true);

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

void ParticleRenderer::exec_fast_render_task(
    Image2D<Spectrum> &target, const Vec2i &beg, const Vec2i &end,
    tracer::Sampler &sampler)
{
    using namespace tracer;

    Arena arena;

    const Camera *camera = scene_->get_camera();

    for(int py = beg.y; py < end.y; ++py)
    {
        for(int px = beg.x; px < end.x; ++px)
        {
            const Sample2 film_sam = sampler.sample2();
            const real pixel_x = px + film_sam.u;
            const real pixel_y = py + film_sam.v;
            const real film_x = pixel_x / target.width();
            const real film_y = pixel_y / target.height();

            const auto cam_ray = camera->sample_we(
                { film_x, film_y }, sampler.sample2());

            const Ray ray(cam_ray.pos_on_cam, cam_ray.pos_to_out);
            const Spectrum radiance = cam_ray.throughput
                                    * fast_render_pixel(*scene_, ray, sampler, arena);

            if(arena.used_bytes() > 4 * 1024 * 1024)
                arena.release();

            target(py, px) = radiance;
        }
    }
}

uint64_t ParticleRenderer::exec_render_task(
    Framebuffer::Task &task, tracer::Sampler &sampler,
    tracer::FilmFilterApplier::FilmGridView<Spectrum> &particle_film)
{
    using namespace tracer;

    uint64_t ret = 0;

    Arena arena;
    const Camera *camera = scene_->get_camera();
    const Rect2i sam_bound = task.pixel_range;

    task.grid->value.clear(Spectrum());
    task.grid->weight.clear(0);

    for(int py = sam_bound.low.y; py <= sam_bound.high.y; ++py)
    {
        for(int px = sam_bound.low.x; px <= sam_bound.high.x; ++px)
        {
            for(int s = 0; s < task.spp; ++s)
            {
                if(stop_rendering_)
                    return ret;

                const Sample2 film_sam = sampler.sample2();
                const real pixel_x = px + film_sam.u;
                const real pixel_y = py + film_sam.v;
                const real film_x = pixel_x / task.full_res.x;
                const real film_y = pixel_y / task.full_res.y;

                auto cam_ray = camera->sample_we(
                    { film_x, film_y }, sampler.sample2());

                const Ray ray(cam_ray.pos_on_cam, cam_ray.pos_to_out);
                const Spectrum radiance = cam_ray.throughput * this->render_pixel(
                    *scene_, ray, sampler, arena, particle_film, &ret);

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

    return ret;
}

AGZ_EDITOR_END
