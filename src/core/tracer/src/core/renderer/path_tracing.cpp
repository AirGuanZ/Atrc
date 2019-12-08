#include <atomic>
#include <thread>
#include <vector>

#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/reporter.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/core/renderer.h>
#include <agz/tracer/factory/raw/renderer.h>
#include <agz/tracer/factory/factory.h>

AGZ_TRACER_BEGIN

class PathTracingRenderer : public Renderer
{
    int worker_count_ = 0;

    std::shared_ptr<const Sampler> sampler_prototype_;
    std::shared_ptr<const PathTracingIntegrator> integrator_;

    static int get_actual_worker_count(int worker_count) noexcept
    {
        if(worker_count > 0)
            return worker_count;
        int hardware_worker_count = static_cast<int>(std::thread::hardware_concurrency());
        return (std::max)(1, hardware_worker_count + worker_count);
    }

    void render_grid(const Scene &scene, Sampler &sampler, FilmGrid &film_grid, const Vec2i &full_res) const
    {
        Arena arena;

        const Camera *cam = scene.camera();
        int x_beg = film_grid.sample_x_beg();
        int x_end = film_grid.sample_x_end();
        int y_beg = film_grid.sample_y_beg();
        int y_end = film_grid.sample_y_end();

        for(int py = y_beg; py < y_end; ++py)
        {
            for(int px = x_beg; px < x_end; ++px)
            {
                sampler.start_pixel(px, py);
                do
                {
                    Sample2 film_sam = sampler.sample2();
                    real pixel_x = px + film_sam.u;
                    real pixel_y = py + film_sam.v;
                    real film_x = pixel_x / full_res.x;
                    real film_y = pixel_y / full_res.y;

                    CameraSample cam_sam = { { film_x, film_y }, sampler.sample2() };
                    auto cam_ray = cam->generate_ray(cam_sam);

                    GBufferPixel gbuffer_pixel;
                    Spectrum f = integrator_->eval(&gbuffer_pixel, scene, cam_ray.r, sampler, arena);
                    Spectrum  value = cam_ray.throughput * f;

                    film_grid.add_sample({ pixel_x, pixel_y }, value, gbuffer_pixel, 1);

                    arena.release();

                } while(sampler.next_sample());
            }
        }
    }

public:

    void initialize(const PathTracingRendererParams &params)
    {
        worker_count_ = params.worker_count;
        sampler_prototype_ = params.sampler_prototype;
        integrator_ = params.integrator;
    }

    void render(Scene &scene, ProgressReporter &reporter, Film *film) override
    {   
        std::atomic<int> next_task_id = 0;
        std::mutex reporter_mutex;
        
        auto func = [
            film,
            &scene,
            &reporter,
            &reporter_mutex,
            &next_task_id,
            total_task_id = film->resolution().y,
            p_this = this
        ] (std::unique_ptr<FilmGrid> &&film_grid, Sampler *sampler)
        {
            Vec2i full_res = film->resolution();

            for(;;)
            {
                int my_task_id = next_task_id++;
                if(my_task_id >= total_task_id)
                    return;

                int x_beg = 0, x_end = full_res.x;
                int y_beg = my_task_id, y_end = my_task_id + 1;
                film_grid = film->renew_grid(x_beg, x_end, y_beg, y_end, std::move(film_grid));

                p_this->render_grid(scene, *sampler, *film_grid, full_res);
                film->merge_grid(*film_grid);

                real percent = real(100) * (my_task_id + 1) / total_task_id;
                std::lock_guard lk(reporter_mutex);
                reporter.progress(percent);
            }
        };

        std::vector<std::thread> threads;
        int actual_worker_count = get_actual_worker_count(worker_count_);
        Arena sampler_arena;

        reporter.begin();
        reporter.new_stage();

        for(int i = 0; i < actual_worker_count; ++i)
        {
            auto film_grid = film->new_grid(0, 1, 0, 1);
            auto sampler = sampler_prototype_->clone(i, sampler_arena);
            threads.emplace_back(func, std::move(film_grid), sampler);
        }

        for(auto &t : threads)
            t.join();

        reporter.end_stage();
        reporter.end();
        
        reporter.message("total time: " + std::to_string(reporter.total_seconds()) + "s");
    }
};

std::shared_ptr<Renderer> create_path_tracing_renderer(
    const PathTracingRendererParams &params)
{
    auto ret = std::make_shared<PathTracingRenderer>();
    ret->initialize(params);
    return ret;
}

AGZ_TRACER_END
