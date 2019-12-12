#include <atomic>
#include <thread>
#include <vector>

#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/reporter.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/core/renderer.h>
#include <agz/tracer/factory/raw/renderer.h>
#include <agz/tracer/factory/factory.h>
#include <agz/utility/thread.h>

AGZ_TRACER_BEGIN

class PathTracingRenderer : public Renderer
{
    int worker_count_   = 0;
    int task_grid_size_ = 32;

    std::shared_ptr<const Sampler>               sampler_prototype_;
    std::shared_ptr<const PathTracingIntegrator> integrator_;

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

                    auto cam_ray = cam->sample_we({ film_x, film_y }, sampler.sample2());

                    GBufferPixel gbuffer_pixel;
                    Ray ray(cam_ray.pos_on_cam, cam_ray.pos_to_out);
                    Spectrum f = integrator_->eval(&gbuffer_pixel, scene, ray, sampler, arena);
                    Spectrum value = cam_ray.throughput * f;

                    film_grid.add_sample({ pixel_x, pixel_y }, value, gbuffer_pixel, 1);

                    arena.release();

                } while(sampler.next_sample());
            }
        }
    }

public:

    void initialize(const PathTracingRendererParams &params)
    {
        worker_count_      = params.worker_count;
        task_grid_size_    = params.task_grid_size;
        sampler_prototype_ = params.sampler_prototype;
        integrator_        = params.integrator;
    }

    void render(Scene &scene, ProgressReporter &reporter, Film *film) override
    {
        film->set_scale(1 / static_cast<real>(sampler_prototype_->get_spp()));

        auto [film_width, film_height] = film->resolution();
        int x_task_count = (film_width  + task_grid_size_ - 1) / task_grid_size_;
        int y_task_count = (film_height + task_grid_size_ - 1) / task_grid_size_;
        int total_task_count = x_task_count * y_task_count;

        std::atomic<int> next_task_id = 0;
        std::mutex reporter_mutex;
        
        auto func = [
            film,
            &scene,
            &reporter,
            &reporter_mutex,
            &next_task_id,
            x_task_count,
            total_task_count,
            task_grid_size = task_grid_size_,
            p_this = this
        ] (Sampler *sampler)
        {
            Vec2i full_res = film->resolution();

            for(;;)
            {
                int my_task_id = next_task_id++;
                if(my_task_id >= total_task_count)
                    return;

                int grid_y_index = my_task_id / x_task_count;
                int grid_x_index = my_task_id % x_task_count;

                int x_beg = grid_x_index * task_grid_size;
                int y_beg = grid_y_index * task_grid_size;
                int x_end = (std::min)(x_beg + task_grid_size, full_res.x);
                int y_end = (std::min)(y_beg + task_grid_size, full_res.y);

                auto film_grid = film->new_grid(x_beg, x_end, y_beg, y_end);

                p_this->render_grid(scene, *sampler, *film_grid, full_res);
                film->merge_grid(*film_grid);

                real percent = real(100) * (my_task_id + 1) / total_task_count;
                std::lock_guard lk(reporter_mutex);
                reporter.progress(percent);
            }
        };

        std::vector<std::thread> threads;
        int actual_worker_count = thread::actual_worker_count(worker_count_);
        Arena sampler_arena;

        scene.start_rendering();

        reporter.begin();
        reporter.new_stage();

        for(int i = 0; i < actual_worker_count; ++i)
        {
            auto sampler = sampler_prototype_->clone(i, sampler_arena);
            threads.emplace_back(func, sampler);
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
