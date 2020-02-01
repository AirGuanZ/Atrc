#include <agz/editor/renderer/ao.h>
#include <agz/tracer/tracer.h>
#include <agz/utility/thread.h>

AGZ_EDITOR_BEGIN

AO::AO(
    const AOParams &params, int fb_width, int fb_height,
    std::shared_ptr<const tracer::Scene> scene)
    : stop_rendering_(false), params_(params), scene_(std::move(scene)),
      framebuffer_(fb_width, fb_height, 32)
{

}

AO::~AO()
{
    stop_rendering_ = true;
    for(auto &t : threads_)
        t.join();
}

void AO::start()
{
    connect(&framebuffer_, &Framebuffer::can_get_img, [=] { emit can_get_img(); });

    auto render_func = [this](tracer::Sampler *sampler)
    {
        std::vector<Framebuffer::Task> tasks;
        for(;;)
        {
            if(stop_rendering_)
                return;

            tasks.clear();
            int task_count = framebuffer_.get_tasks(1, tasks);

            for(int i = 0; i < task_count; ++i)
                exec_render_task(tasks[i], sampler);

            framebuffer_.merge_tasks(task_count, tasks.data());
        }
    };

    auto sampler_prototype = tracer::create_native_sampler(1, 0, true);

    int worker_count = thread::actual_worker_count(params_.worker_count);
    for(int i = 0; i < worker_count; ++i)
    {
        auto sampler = sampler_prototype->clone(i, sampler_arena_);
        threads_.emplace_back(render_func, sampler);
    }

    framebuffer_.start();
}

Image2D<math::color3b> AO::get_image() const
{
    return framebuffer_.get_image();
}

void AO::exec_render_task(Framebuffer::Task &task, tracer::Sampler *sampler)
{
    using namespace tracer;

    const render::AOParams ao_params = {
        params_.background_color,
        params_.low_color,
        params_.high_color,
        params_.ao_sample_count,
        params_.occlusion_distance
    };

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
                const Spectrum radiance = trace_ao(ao_params, *scene_, ray, *sampler).value;

                if(radiance.is_finite())
                {
                    const int lx = px - sam_bound.low.x;
                    const int ly = py - sam_bound.low.y;

                    task.grid->value(ly, lx) += radiance;
                    task.grid->weight(ly, lx) += 1;
                }

            }
        }
    }
}

AGZ_EDITOR_END
