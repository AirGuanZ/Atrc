#include <atomic>
#include <chrono>
#include <mutex>
#include <queue>
#include <thread>

#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/guided_pt_integrator.h>
#include <agz/tracer/core/renderer.h>
#include <agz/tracer/core/reporter.h>
#include <agz/tracer/core/sampler.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/utility/grid_divider.h>
#include <agz/utility/thread.h>

#include "./guider.h"

/*
    参考 http://drz.disneyresearch.com/~jnovak/publications/PathGuide/index.html
        renderer开一个scheduler线程以支持render_async，scheduler开一个recorder线程
        每轮训练中scheduler开若干工作线程，工作线程将record收集成batch提交给recorder
*/

AGZ_TRACER_BEGIN

namespace pgpt
{

class GuidedPathTracer : public Renderer
{
    using Divider = GridDivider<int>;
    using Grid = Divider::Grid;

    Scene            *scene_;
    ProgressReporter *reporter_;
    Film             *film_;

    std::unique_ptr<Guider> guider_;

    std::thread scheduler_thread_;

    std::unique_ptr<thread::queue_executer_t<Grid>> executor_;

    size_t total_task_count_;
    std::atomic<size_t> finished_task_count_;

    const Sampler *sampler_prototype_;
    std::mutex sampler_prototype_mutex_;
    std::mutex film_merge_mutex_;

    GuidedPathTracingIntegrator *integrator_;

    int initial_spp_;
    real iterate_c_;
    real iterate_rho_;

    int rendering_spp_;

    int training_count_;

    int task_grid_size_;
    int rendering_worker_count_;

    size_t record_batch_size_;

    void clear()
    {
        scene_             = nullptr;
        reporter_          = nullptr;
        film_              = nullptr;
        guider_            = nullptr;
    }

    template<bool TrainGuider, bool SampleGuider>
    void render_grid(const Scene &scene, Sampler &sampler, Guider &guider, FilmGrid *film_grid, const Vec2i &full_res, int spp)
    {
        Arena arena;

        RecordBatchBuilder batch_builder(record_batch_size_, guider);

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
                for(int s = 0; s < spp; ++s)
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

                    auto value = integrator_->eval(
                        scene, cam_ray.r, sampler, arena,
                        batch_builder, guider, TrainGuider, SampleGuider);
                    real we_cos = std::abs(cos(cam_ray.r.d, cam_ray.nor));
                    value *= we_cos * cam_ray.importance / (cam_ray.pdf_pos * cam_ray.pdf_dir);

                    if constexpr(!TrainGuider)
                        film_grid->add_sample({ pixel_x, pixel_y }, value, GBufferPixel(), 1);
                    arena.release();
                }
            }
        }

        batch_builder.end();
    }

    template<bool TrainGuider, bool SampleGuider>
    void perform_render_pass(int spp)
    {
        auto res = film_->resolution();
        std::queue<Grid> tasks;

        Grid full_grid = { 0, res.x, 0, res.y };
        Divider::divide(full_grid, task_grid_size_, task_grid_size_, misc::push_inserter(tasks));

        total_task_count_ = tasks.size();
        finished_task_count_ = 0;

        auto func = [pt = this, res = res, spp = spp](const Grid &grid)
        {
            Arena arena;

            int task_id = grid.x_begin * static_cast<int>(pt->total_task_count_) + grid.y_begin;
            Sampler *grid_sampler;
            {
                std::lock_guard lk(pt->sampler_prototype_mutex_);
                grid_sampler = pt->sampler_prototype_->clone(task_id, arena);
            }

            auto film_grid = pt->film_->new_grid(grid.x_begin, grid.x_end, grid.y_begin, grid.y_end);
            pt->render_grid<TrainGuider, SampleGuider>(
                *pt->scene_, *grid_sampler, *pt->guider_, film_grid.get(), res, spp);

            std::lock_guard lk(pt->film_merge_mutex_);

            if constexpr(!TrainGuider)
                pt->film_->merge_grid(std::move(*film_grid));

            real percent = real(100) * ++pt->finished_task_count_ / pt->total_task_count_;
            pt->reporter_->progress(percent);
        };

        executor_ = std::make_unique<thread::queue_executer_t<Grid>>();
        reporter_->new_stage();

        executor_->run_async(rendering_worker_count_, std::move(tasks), func);
        executor_->join();

        reporter_->end_stage();

        executor_ = nullptr;
    }

    static void async_scheduler(GuidedPathTracer *pt)
    {
        auto world_bound = pt->scene_->world_bound();
        Vec3 world_diag = world_bound.high - world_bound.low;
        world_bound.low  -= real(0.1) * world_diag;
        world_bound.high += real(0.1) * world_diag;
        pt->guider_ = std::make_unique<Guider>(world_bound);

        int spp   = pt->initial_spp_;
        int pow_k = 1;
        pt->perform_render_pass<true, false>(spp);

        if(pt->training_count_ > 1)
        {
            int stree_threshold = static_cast<int>(pt->iterate_c_ * std::sqrt(pow_k));
            pt->guider_->iterate(pt->iterate_rho_, stree_threshold);
        }
        else
            pt->guider_->iterate_end();

        for(int i = 1; i < pt->training_count_; ++i)
        {
            spp   += spp;
            pow_k += pow_k;
            pt->perform_render_pass<true, true>(spp);

            if(i < pt->training_count_ - 1)
            {
                int stree_threshold = static_cast<int>(pt->iterate_c_ * std::sqrt(pow_k));
                pt->guider_->iterate(pt->iterate_rho_, stree_threshold);
            }
            else
                pt->guider_->iterate_end();
        }

        int final_spp = pt->rendering_spp_ > 0 ? pt->rendering_spp_ : spp + spp;
        pt->perform_render_pass<false, true>(final_spp);
    }

public:

    explicit GuidedPathTracer()
    {
        scene_             = nullptr;
        sampler_prototype_ = nullptr;
        reporter_          = nullptr;
        film_              = nullptr;

        finished_task_count_ = 0;
        total_task_count_ = 0;

        integrator_ = nullptr;

        initial_spp_ = 4;
        iterate_c_ = 1000;
        iterate_rho_ = real(0.01);

        rendering_spp_ = 0;

        training_count_ = 3;

        task_grid_size_ = 32;
        rendering_worker_count_ = -1;

        record_batch_size_ = 10000;
    }

    static std::string description()
    {
        return R"___(
guided_pt [Renderer]
    record_batch_size      [int]  (optional; default: 10000) batch size when committing radiance records
    k0                     [int]  initial training spp
    c                      [real] constant c in computing subdivision threshold of spatial tree
    rho                    [real] constant rho in computing subdivision threshold of directional tree
    training_count         [int]  how many training iteration to perform
    task_grid_size         [int]  (optional; default: 32) grid size in dividing rendering target
    rendering_worker_count [int]  rendering thread count
    integrator             [GuidedPathTracingIntegrator] integrator object
    rendering_spp          [int]  (optional) final rendering spp
    sampler                [Sampler] sampler prototype

    gbuffer is unsupported
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        initial_spp_ = params.child_int("k0");
        if(initial_spp_ < 1)
            throw ObjectConstructionException("invalid k0 value: " + std::to_string(initial_spp_));

        iterate_c_ = params.child_real("c");
        if(iterate_c_ < 1)
            throw ObjectConstructionException("invalid c value: " + std::to_string(iterate_c_));

        iterate_rho_ = params.child_real("rho");
        if(iterate_rho_ <= 0 || iterate_rho_ > 1)
            throw ObjectConstructionException("invalid rho value: " + std::to_string(iterate_rho_));

        training_count_ = params.child_int("training_count");
        if(training_count_ < 1)
            throw ObjectConstructionException("invalid training count: " + std::to_string(training_count_));

        rendering_worker_count_ = params.child_int("rendering_worker_count");

        if(auto node = params.find_child("task_grid_size"))
        {
            task_grid_size_ = node->as_value().as_int();
            if(task_grid_size_ <= 0)
                throw ObjectConstructionException("invalid task grid size value: " + std::to_string(task_grid_size_));
        }

        if(auto node = params.find_child("batch_size"))
        {
            record_batch_size_ = node->as_value().as_int();
            if(record_batch_size_ <= 0)
                throw ObjectConstructionException("invalid record batch size value: " + std::to_string(record_batch_size_));
        }

        if(auto node = params.find_child("rendering_spp"))
            rendering_spp_ = node->as_value().as_int();

        integrator_ = GuidedPathTracingIntegratorFactory.create(params.child_group("integrator"), init_ctx);

        sampler_prototype_ = SamplerFactory.create(params.child_group("sampler"), init_ctx);

        AGZ_HIERARCHY_WRAP("in initializing guided path tracer")
    }

    void render_async(Scene &scene, ProgressReporter &reporter, Film *film) override
    {
        assert(!scheduler_thread_.joinable());

        scene_             = &scene;
        reporter_          = &reporter;
        film_              = film;

        reporter_->begin();

        scheduler_thread_ = std::thread(async_scheduler, this);
    }

    void join() override
    {
        assert(scheduler_thread_.joinable());
        scheduler_thread_.join();

        reporter_->end();

        clear();
    }

    void stop() override
    {
        join();
    }
};

AGZT_IMPLEMENTATION(Renderer, GuidedPathTracer, "guided_pt")

} // namespace pgpt

AGZ_TRACER_END
