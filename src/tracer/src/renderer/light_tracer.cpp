#include <unordered_map>

#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/renderer.h>
#include <agz/tracer/core/reporter.h>
#include <agz/tracer/core/sampler.h>
#include <agz/tracer/core/scene.h>
#include <agz/utility/thread.h>

AGZ_TRACER_BEGIN

class LightTracer : public Renderer
{
    struct Task
    {
        int sample_count;
        int id;
    };

    int worker_count_ = 0;
    int particle_count_ = 10000;

    int min_depth_ = 1;
    int max_depth_ = 10;
    real cont_prob_ = 1;

    int total_task_count_ = 1;
    std::atomic<int> finished_task_count_ = 0;

    const Sampler *sampler_prototype_ = nullptr;
    std::mutex sampler_prototype_mutex_;

    Film *film_ = nullptr;

    std::mutex reporter_mutex_;
    ProgressReporter *reporter_ = nullptr;

    std::mutex thread_films_mutex_;
    std::unordered_map<std::thread::id, std::unique_ptr<FilmGrid>> thread_films_;

    std::unique_ptr<thread::queue_executer_t<Task>> executor_;

    void light_tracing_eval(
        const Scene &scene, Sampler &sampler,
        FilmGrid *full_film_grid, const Vec2i &film_res,
        const Vec2 &film_coord_low, const Vec2 &film_coord_high,
        Arena &arena) const
    {
        auto [emitting_light, select_light_pdf] = scene.sample_light(sampler.sample1());
        if(!emitting_light)
            return;

        auto emit_result = emitting_light->emit(sampler.sample5());
        if(!emit_result.radiance)
            return;

        Spectrum coef(1 / (emit_result.pdf_pos * select_light_pdf));
        auto camera = scene.camera();

        auto on_film = [=](const Vec2 &film_coord)
        {
            return film_coord_low.x <= film_coord.x && film_coord.x <= film_coord_high.x &&
                   film_coord_low.y <= film_coord.y && film_coord.y <= film_coord_high.y;
        };

        auto to_pixel = [=](const Vec2 &film_coord)
        {
            return Vec2(film_coord.x * film_res.x, film_coord.y * film_res.y);
        };

        auto cam_direct_sample = camera->sample(emit_result.spt.pos, sampler.sample2());
        if(cam_direct_sample.importance != 0 && on_film(cam_direct_sample.film_coord))
        {
            real f = std::abs(cos(emit_result.spt.geometry_coord.z, cam_direct_sample.ref_to_cam));
            auto radiance = emitting_light->radiance(emit_result.spt, cam_direct_sample.ref_to_cam);;
            if(!radiance.is_black())
            {
                real dist = (emit_result.spt.pos - cam_direct_sample.film_pos).length();
                Ray shadow_ray(emit_result.spt.pos, cam_direct_sample.ref_to_cam.normalize(), EPS, dist - EPS);
                if(dist > EPS && !scene.has_intersection(shadow_ray))
                {
                    Spectrum path_contrib = coef * f * radiance * cam_direct_sample.importance / cam_direct_sample.pdf;
                    full_film_grid->add_sample(to_pixel(cam_direct_sample.film_coord), path_contrib, GBufferPixel(), 1);
                }
            }
        }

        coef *= emit_result.radiance * std::abs(cos(emit_result.spt.geometry_coord.z, emit_result.dir))
                / emit_result.pdf_dir;
        Ray r(emit_result.spt.pos, emit_result.dir.normalize(), EPS);

        for(int depth = 1; depth < max_depth_; ++depth)
        {
            if(depth > min_depth_)
            {
                if(sampler.sample1().u > cont_prob_)
                    return;
                coef /= cont_prob_;
            }

            EntityIntersection inct;
            if(!scene.closest_intersection(r, &inct))
                return;

            auto shd = inct.material->shade(inct, arena);

            auto camera_sample = camera->sample(inct.pos, sampler.sample2());
            if(camera_sample.importance != 0 && on_film(camera_sample.film_coord))
            {
                real dist = (inct.pos - camera_sample.film_pos).length();
                Ray shadow_ray(inct.pos, (camera_sample.film_pos - inct.pos).normalize(), EPS, dist - EPS);
                if(dist > EPS && !scene.has_intersection(shadow_ray))
                {
                    auto f = shd.bsdf->eval(
                        camera_sample.ref_to_cam.normalize(),
                        inct.wr.normalize(),
                        TM_Importance);
                    if(!f.is_black())
                    {
                        real proj_factor = std::abs(cos(camera_sample.ref_to_cam, inct.geometry_coord.z));
                        Spectrum path_contrib = coef * f * proj_factor * camera_sample.importance / camera_sample.pdf;
                        full_film_grid->add_sample(to_pixel(camera_sample.film_coord), path_contrib, GBufferPixel(), 1);
                    }
                }
            }

            auto bsdf_sample = shd.bsdf->sample(inct.wr, TM_Importance, sampler.sample3());
            if(!bsdf_sample.f || !bsdf_sample.pdf)
                return;
            coef *= bsdf_sample.f * std::abs(cos(bsdf_sample.dir, inct.geometry_coord.z)) / bsdf_sample.pdf;

            Vec3 true_nor = inct.geometry_coord.z;
            if(dot(bsdf_sample.dir, inct.geometry_coord.z) < 0)
                true_nor = -true_nor;
            Vec3 new_pos = inct.pos + EPS * true_nor;
            r = Ray(new_pos, bsdf_sample.dir.normalize());
        }
    }

    void render_task(
        const Scene &scene, int sample_count, Sampler &sampler,
        FilmGrid *full_film_grid, const Vec2i &film_res,
        const Vec2 &film_coord_low, const Vec2 &film_coord_high) const
    {
        Arena arena;
        for(int sample_idx = 0; sample_idx < sample_count; ++sample_idx)
        {
            sampler.start_pixel(sample_idx, 0);
            light_tracing_eval(scene, sampler, full_film_grid, film_res, film_coord_low, film_coord_high, arena);
            arena.release();
        }
    }

public:

    using Renderer::Renderer;

    static std::string description()
    {
        return R"___(
light [Renderer]
    worker_count   [int] rendering thread count; non-positive -> hardware thread count
    min_depth      [int] minimum path length before applying Russian Roulette
    max_depth      [int] maximal path length
    cont_prob      [real] continuing probability in Russian Roulette
    particle_count [int] traced particle count
    sampler        [Sampler] sampler prototype

    gbuffer is unsupported
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        worker_count_ = params.child_int("worker_count");
        min_depth_ = params.child_int("min_depth");
        if(min_depth_ < 1)
            throw ObjectConstructionException("invalid min depth value: " + std::to_string(min_depth_));

        max_depth_ = params.child_int("max_depth");
        if(max_depth_ < min_depth_)
            throw ObjectConstructionException("invalid max depth value: " + std::to_string(max_depth_));

        cont_prob_ = params.child_real("cont_prob");
        if(cont_prob_ < 0 || cont_prob_ > 1)
            throw ObjectConstructionException("invalid continue prob value: " + std::to_string(cont_prob_));

        particle_count_ = params.child_int("particle_count");
        if(particle_count_ < 1)
            throw ObjectConstructionException("invalid particle count value: " + std::to_string(particle_count_));

        sampler_prototype_ = SamplerFactory.create(params.child_group("sampler"), init_ctx);

        AGZ_HIERARCHY_WRAP("in initializing light tracer")
    }

    void render_async(Scene &scene, ProgressReporter &reporter, Film *film) override
    {
        executor_ = nullptr;
        thread_films_.clear();

        std::queue<Task> tasks;
        int dem = thread::queue_executer_t<Task>::actual_worker_count(worker_count_);
        int task_size = std::max(1000, particle_count_ / dem / 16);

        int complete_task_count = particle_count_ / task_size;
        for(int i = 0; i < complete_task_count; ++i)
            tasks.push({ task_size, i });

        int rest_task_size = particle_count_ % task_size;
        if(rest_task_size)
            tasks.push({ rest_task_size, complete_task_count });

        total_task_count_ = static_cast<int>(tasks.size());
        finished_task_count_ = 0;

        film_ = film;

        auto func = [p_this = this,
            &sampler_prototype = *sampler_prototype_,
            &scene = scene,
            &reporter = reporter,
            res = film->resolution()]
            (const Task &task)
        {
            Arena arena;

            Sampler *task_sampler;
            {
                std::lock_guard lk(p_this->sampler_prototype_mutex_);
                task_sampler = sampler_prototype.clone(task.id, arena);
            }

            auto thread_id = std::this_thread::get_id();
            FilmGrid *film_grid;
            {
                std::lock_guard lk(p_this->thread_films_mutex_);
                auto it = p_this->thread_films_.find(thread_id);
                if(it == p_this->thread_films_.end())
                {
                    auto new_film = p_this->film_->new_grid(0, res.x, 0, res.y);
                    film_grid = new_film.get();
                    p_this->thread_films_[thread_id] = std::move(new_film);
                }
                else
                    film_grid = it->second.get();
            }

            Vec2 film_coord_low(
                static_cast<real>(film_grid->sample_x_beg()) / res.x,
                static_cast<real>(film_grid->sample_y_beg()) / res.y);
            Vec2 film_coord_high(
                static_cast<real>(film_grid->sample_x_end()) / res.x,
                static_cast<real>(film_grid->sample_y_end()) / res.y);
            p_this->render_task(scene, task.sample_count, *task_sampler, film_grid, res, film_coord_low, film_coord_high);

            std::lock_guard lk(p_this->reporter_mutex_);

            real percent = real(100) * ++p_this->finished_task_count_ / p_this->total_task_count_;
            reporter.progress(percent);
        };

        executor_ = std::make_unique<thread::queue_executer_t<Task>>();
        reporter_ = &reporter;

        reporter.begin();
        reporter.new_stage();

        executor_->run_async(worker_count_, std::move(tasks), func);
    }

    void join() override
    {
        assert(executor_ && executor_->running());
        executor_->join();

        for(auto &p : thread_films_)
            film_->merge_grid(std::move(*p.second));

        real ratio = film_->resolution().product() / real(particle_count_);
        film_->map_spectrum([=](const Spectrum &c) { return ratio * c; });
        film_->map_weight([](real w) { return real(1); });

        film_ = nullptr;
        thread_films_.clear();

        assert(reporter_);
        reporter_->end_stage();
        reporter_->end();
        reporter_->message("Total time: " + std::to_string(reporter_->total_seconds()) + "s");
        reporter_ = nullptr;

        executor_ = nullptr;
    }

    void stop() override
    {
        assert(executor_ && executor_->running());
        executor_->stop();

        film_ = nullptr;
        thread_films_.clear();

        assert(reporter_);
        reporter_->end_stage();
        reporter_->end();
        reporter_ = nullptr;

        executor_ = nullptr;
    }
};

AGZT_IMPLEMENTATION(Renderer, LightTracer, "light")

AGZ_TRACER_END