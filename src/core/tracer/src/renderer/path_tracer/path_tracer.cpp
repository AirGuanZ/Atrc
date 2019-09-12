#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/path_tracing_integrator.h>
#include <agz/tracer/core/renderer.h>
#include <agz/tracer/core/reporter.h>
#include <agz/tracer/core/sampler.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/utility/grid_divider.h>
#include <agz/utility/misc.h>
#include <agz/utility/thread.h>

AGZ_TRACER_BEGIN

class PathTracer : public Renderer
{
    PathTracingIntegrator *integrator_ = nullptr;
    int worker_count                   = 0;

    int task_grid_size_ = 32;

    using Divider = GridDivider<int>;
    using Grid = Divider::Grid;

    std::unique_ptr<thread::queue_executer_t<Grid>> executer_;

    size_t total_task_count_ = 1;
    std::atomic<size_t> finished_task_count_ = 0;

    const Sampler *sampler_prototype_ = nullptr;
    std::mutex sampler_prototype_mutex_;
    std::mutex film_merge_mutex_;

    ProgressReporter *reporter_ = nullptr;

    void render_grid(const Scene &scene, Sampler &sampler, FilmGrid *film_grid, const Vec2i &full_res) const
    {
        Arena arena;

        auto cam = scene.camera();
        auto x_beg = film_grid->sample_x_beg();
        auto x_end = film_grid->sample_x_end();
        auto y_beg = film_grid->sample_y_beg();
        auto y_end = film_grid->sample_y_end();

        for(int py = y_beg; py < y_end; ++py)
        {
            for(int px = x_beg; px < x_end; ++px)
            {
                sampler.start_pixel(px, py);
                do
                {
                    auto film_sam = sampler.sample2();

                    real pixel_x = px + film_sam.u;
                    real pixel_y = py + film_sam.v;

                    real film_x = static_cast<real>(pixel_x) / full_res.x;
                    real film_y = static_cast<real>(pixel_y) / full_res.y;

                    CameraSample cam_sam = { { film_x, film_y }, sampler.sample2() };
                    auto cam_ray = cam->generate_ray(cam_sam);
                    if(cam_ray.pdf_pos < EPS || cam_ray.pdf_dir < EPS)
                        continue;

                    GBufferPixel gpixel;
                    auto f = integrator_->eval(&gpixel, scene, cam_ray.r, sampler, arena);

                    real we_cos = std::abs(cos(cam_ray.r.d, cam_ray.nor));
                    auto value = we_cos * f * cam_ray.importance / (cam_ray.pdf_pos * cam_ray.pdf_dir);

                    film_grid->add_sample({ pixel_x, pixel_y }, value, gpixel, 1);
                    arena.release();

                } while(sampler.next_sample());
            }
        }
    }

public:

    using Renderer::Renderer;

    ~PathTracer()
    {
        executer_ = nullptr;
    }

    static std::string description()
    {
        return R"___(
pt [Renderer]
    integrator     [PathTracingIntegrator] path tracing integrator
    worker_count   [int] rendering thread count; non-positive -> hardware thread count
    task_grid_size [int] (optional) thread task grid size (defaultly 32)
    sampler        [Sampler] sampler prototype

    gbuffer is supported
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &context) override
    {
        AGZ_HIERARCHY_TRY

        init_customed_flag(params);

        integrator_ = PathTracingIntegratorFactory.create(params.child_group("integrator"), context);
        worker_count = params.child_int("worker_count");

        if(auto node = params.find_child("task_grid_size"))
        {
            task_grid_size_ = node->as_value().as_int();
            if(task_grid_size_ <= 0)
                throw ObjectConstructionException("invalid task grid size value: " + std::to_string(task_grid_size_));
        }

        sampler_prototype_ = SamplerFactory.create(params.child_group("sampler"), context);

        AGZ_HIERARCHY_WRAP("in initializing path tracer")
    }

    void render_async(Scene &scene, ProgressReporter &reporter, Film *film) override
    {
        executer_ = nullptr;

        auto res = film->resolution();
        std::queue<Grid> tasks;

        Grid full_grid = { 0, res.x, 0, res.y };
        Divider::divide(full_grid, task_grid_size_, task_grid_size_, misc::push_inserter(tasks));

        total_task_count_ = tasks.size();
        finished_task_count_ = 0;

        auto func = [p_this    = this,
                     &sampler  = *sampler_prototype_,
                     &scene    = scene,
                     &reporter = reporter,
                     film      = film,
                     res       = res]
            (const Grid &grid)
        {
            Arena arena;

            int task_id = grid.x_begin * static_cast<int>(p_this->total_task_count_) + grid.y_begin;
            Sampler *grid_sampler;
            {
                std::lock_guard lk(p_this->sampler_prototype_mutex_);
                grid_sampler = sampler.clone(task_id, arena);
            }

            auto film_grid = film->new_grid(grid.x_begin, grid.x_end, grid.y_begin, grid.y_end);
            p_this->render_grid(scene, *grid_sampler, film_grid.get(), res);

            std::lock_guard lk(p_this->film_merge_mutex_);
            
            film->merge_grid(std::move(*film_grid));

            real percent = real(100) * ++p_this->finished_task_count_ / p_this->total_task_count_;
            reporter.progress(percent);
        };

        executer_ = std::make_unique<thread::queue_executer_t<Grid>>();
        reporter_ = &reporter;

        reporter.begin();
        reporter.new_stage();

        executer_->run_async(worker_count, std::move(tasks), func);
    }

    void join() override
    {
        assert(executer_ && executer_->running());
        executer_->join();

        assert(reporter_);
        reporter_->end_stage();
        reporter_->end();

        auto exceptions = executer_->exceptions();
        for(auto &ptr : exceptions)
            reporter_->message("Exception: " + misc::extract_exception_ptr(ptr));

        reporter_->message("Total time: " + std::to_string(reporter_->total_seconds()) + "s");
        reporter_ = nullptr;

        executer_ = nullptr;
    }

    void stop() override
    {
        assert(executer_ && executer_->running());
        executer_->stop();

        assert(reporter_);
        reporter_->end_stage();
        reporter_->end();

        auto exceptions = executer_->exceptions();
        for(auto &ptr : exceptions)
            reporter_->message("Exception: " + misc::extract_exception_ptr(ptr));

        reporter_ = nullptr;
        executer_ = nullptr;
    }
};

AGZT_IMPLEMENTATION(Renderer, PathTracer, "pt")

AGZ_TRACER_END
