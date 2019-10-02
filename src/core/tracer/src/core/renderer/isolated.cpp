#include <agz/tracer/core/bssrdf.h>
#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/renderer.h>
#include <agz/tracer/core/reporter.h>
#include <agz/tracer/core/sampler.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/utility/grid_divider.h>
#include <agz/utility/misc.h>
#include <agz/utility/thread.h>

#include <agz/tracer/factory/raw/renderer.h>

#include "path_tracer/mis_light_bsdf.h"

AGZ_TRACER_BEGIN

class IsolatedPathTracer : public Renderer
{
    int min_depth_ = 5;
    int max_depth_ = 10;
    real cont_prob_ = real(0.9);

    int shading_aa_ = 1;
    int background_aa_ = 1;

    Spectrum background_color_ = Spectrum(1);
    bool hide_background_entity_ = false;

    int worker_count_ = 0;

    int task_grid_size_ = 32;

    using Divider = GridDivider<int>;
    using Grid = Divider::Grid;

    std::unique_ptr<thread::queue_executer_t<Grid>> executer_;

    size_t total_task_count_ = 1;
    std::atomic<size_t> finished_task_count_ = 0;

    std::shared_ptr<const Sampler> sampler_prototype_;
    std::mutex sampler_prototype_mutex_;
    std::mutex film_merge_mutex_;

    ProgressReporter *reporter_ = nullptr;

    Spectrum sample_bsdf(
        const Scene &scene, const EntityIntersection &inct, const ShadingPoint &shd, Sampler &sampler,
        Spectrum *coef_delta, bool *has_new_inct, EntityIntersection *new_inct) const
    {
        auto sam3 = sampler.sample3();

        *has_new_inct = false;
        auto bsdf_sample = shd.bsdf->sample(inct.wr, TM_Radiance, sam3);
        if(!bsdf_sample.f || bsdf_sample.pdf < EPS)
            return {};

        Ray new_ray(inct.pos, bsdf_sample.dir.normalize(), EPS);
        if(scene.closest_intersection(new_ray, new_inct))
        {
            *has_new_inct = true;
            *coef_delta = bsdf_sample.f * std::abs(cos(inct.geometry_coord.z, bsdf_sample.dir)) / bsdf_sample.pdf;

            if(auto light = new_inct->entity->as_light())
            {
                Spectrum light_f = light->radiance(*new_inct, new_inct->wr);
                if(!light_f)
                    return {};

                Spectrum f = light_f * bsdf_sample.f * std::abs(cos(inct.geometry_coord.z, bsdf_sample.dir));
                if(bsdf_sample.is_delta)
                    return f / bsdf_sample.pdf;

                real light_pdf = light->pdf(inct.pos, *new_inct);
                return f / (bsdf_sample.pdf + light_pdf);
            }
        }
        else
        {
            auto light = scene.env();
            Spectrum light_f = light->radiance(inct.pos, new_ray.d);

            Spectrum f = light_f * bsdf_sample.f * std::abs(cos(inct.geometry_coord.z, bsdf_sample.dir));
            if(bsdf_sample.is_delta)
                return f / bsdf_sample.pdf;

            real light_pdf = light->pdf(inct.pos, bsdf_sample.dir);
            return f / (bsdf_sample.pdf + light_pdf);
        }

        return {};
    }

    Spectrum trace_background_entity(const Scene &scene, const Ray &ray, const EntityIntersection &inct, Sampler &sampler, int sample_count) const
    {
        Spectrum ret(0);
        for(int i = 0; i < sample_count; ++i)
        {
            Sample2 sam2 = sampler.sample2();
            Vec3 local_dir = math::distribution::zweighted_on_hemisphere(sam2.u, sam2.v).first;
            Vec3 global_dir = inct.geometry_coord.local_to_global(local_dir).normalize();
            Ray r(inct.pos, global_dir, EPS);
            if(!scene.has_intersection(r))
                ret += Spectrum(1);
        }
        Spectrum env_factor = hide_background_entity_ ? scene.env()->radiance(ray.o, ray.d) : Spectrum(1);
        return real(1) / sample_count * ret * env_factor;
    }

    Spectrum trace2(const Scene &scene, EntityIntersection inct, ShadingPoint shd, Sampler &sampler, Arena &arena) const
    {
        Spectrum ret(0), coef(1);

        if(auto lht = inct.entity->as_light())
            ret += lht->radiance(inct, inct.wr);

        for(int depth = 2; depth <= max_depth_; ++depth)
        {
            if(depth > min_depth_)
            {
                if(sampler.sample1().u > cont_prob_)
                    break;
                ret /= cont_prob_;
            }

            for(auto light : scene.lights())
                ret += coef * mis_sample_light(scene, light, inct, shd, sampler.sample5());

            ScatteringPoint pnt(inct, shd.bsdf, shd.bssrdf);
            ret += coef * mis_sample_scattering(scene, pnt, sampler.sample3());

            auto bsdf_sample = shd.bsdf->sample(inct.wr, TM_Radiance, sampler.sample3());
            if(!bsdf_sample.f || bsdf_sample.pdf < EPS)
                break;

            Ray r(pnt.pos(), bsdf_sample.dir.normalize(), EPS);
            coef *= bsdf_sample.f * pnt.proj_wi_factor(bsdf_sample.dir) / bsdf_sample.pdf;
            if((std::max)({ coef.r, coef.g, coef.b }) < real(0.001))
                break;

            bool use_bssrdf = false;
            if(shd.bssrdf)
            {
                bool wi_up = inct.geometry_coord.in_positive_z_hemisphere(bsdf_sample.dir);
                bool wo_up = inct.geometry_coord.in_positive_z_hemisphere(inct.wr);
                if(wo_up && !wi_up)
                    use_bssrdf = true;
            }

            if(use_bssrdf)
            {
                auto bssrdf_sample = shd.bssrdf->sample(bsdf_sample.dir, TM_Radiance, sampler.sample4(), arena);
                if(!bssrdf_sample.valid() || !bssrdf_sample.f)
                    break;
                coef *= bssrdf_sample.f / bssrdf_sample.pdf;

                auto new_pnt = ScatteringPoint(bssrdf_sample.inct, bssrdf_sample.bsdf);
                for(auto light : scene.lights())
                    ret += coef * mis_sample_light(scene, light, new_pnt, sampler.sample5());
                ret += coef * mis_sample_scattering(scene, new_pnt, sampler.sample3());

                BSDFSampleResult new_bsdf_sample = new_pnt.sample(new_pnt.wr(), sampler.sample3(), TM_Radiance);
                if(!new_bsdf_sample.f || new_bsdf_sample.pdf < EPS)
                    break;

                r = Ray(new_pnt.pos(), new_bsdf_sample.dir.normalize(), EPS);
                coef *= new_bsdf_sample.f * new_pnt.proj_wi_factor(new_bsdf_sample.dir) / new_bsdf_sample.pdf;
                if((std::max)({ coef.r, coef.g, coef.b }) < real(0.001))
                    break;
            }

            if(!scene.closest_intersection(r, &inct))
                break;
            shd = inct.material->shade(inct, arena);
        }

        return ret;
    }

    Spectrum trace(const Scene &scene, EntityIntersection inct, ShadingPoint shd, Sampler &sampler, Arena &arena) const
    {
        Spectrum ret(0), coef(1);

        if(auto lht = inct.entity->as_light())
            ret += lht->radiance(inct, inct.wr);

        for(int depth = 2; depth <= max_depth_; ++depth)
        {
            if(depth > min_depth_)
            {
                if(sampler.sample1().u > cont_prob_)
                    return ret;
                coef /= cont_prob_;
            }

            for(auto light : scene.lights())
                ret += coef * mis_sample_light(scene, light, inct, shd, sampler.sample5());

            EntityIntersection new_inct;
            Spectrum coef_delta;
            bool has_new_inct;
            ret += coef * sample_bsdf(scene, inct, shd, sampler, &coef_delta, &has_new_inct, &new_inct);

            coef *= coef_delta;

            if(!has_new_inct)
                break;

            if(new_inct.entity->is_shadow_catcher())
            {
                ret += coef * trace_background_entity(scene, Ray(inct.pos, -new_inct.wr, EPS), new_inct, sampler, 1);
                return ret;
            }
            inct = new_inct;
            shd = inct.material->shade(inct, arena);
        }

        return ret;
    }

    std::pair<Spectrum, bool> eval(GBufferPixel *gpixel, const Scene &scene, const Ray &ray, Sampler &sampler, Arena &arena) const
    {
        EntityIntersection inct;
        if(!scene.closest_intersection(ray, &inct))
            return { scene.env()->radiance(ray.o, ray.d), false };

        ShadingPoint shd = inct.material->shade(inct, arena);
        gpixel->albedo = shd.bsdf->albedo();
        gpixel->normal = inct.user_coord.z;

        if(inct.entity->is_shadow_catcher())
            return { trace_background_entity(scene, ray, inct, sampler, background_aa_), false };

        Spectrum ret(0);
        for(int i = 0; i < shading_aa_; ++i)
            ret += trace(scene, inct, shd, sampler, arena);
        return { real(1) / shading_aa_ * ret, true };
    }

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
                int total_spp = 0;
                int max_spp = 5;

                static Sample2 fst_5_film_sams[] =
                {
                    { 0.5f, 0.5f },
                    { 0.25f, 0.25f },
                    { 0.75f, 0.25f },
                    { 0.25f, 0.75f },
                    { 0.75f, 0.75f }
                };
                int sample_count = 0;

                do
                {
                    auto film_sam = sample_count >= 5 ? sampler.sample2() : fst_5_film_sams[sample_count++];

                    real pixel_x = px + film_sam.u;
                    real pixel_y = py + film_sam.v;

                    real film_x = static_cast<real>(pixel_x) / full_res.x;
                    real film_y = static_cast<real>(pixel_y) / full_res.y;

                    CameraSample cam_sam = { { film_x, film_y }, sampler.sample2() };
                    auto cam_ray = cam->generate_ray(cam_sam);
                    if(cam_ray.pdf_pos < EPS || cam_ray.pdf_dir < EPS)
                        continue;

                    GBufferPixel gpixel;
                    auto [f, prod] = eval(&gpixel, scene, cam_ray.r, sampler, arena);

                    real we_cos = std::abs(cos(cam_ray.r.d, cam_ray.nor));
                    auto value = we_cos * f * cam_ray.importance / (cam_ray.pdf_pos * cam_ray.pdf_dir);

                    film_grid->add_sample({ pixel_x, pixel_y }, value, gpixel, 1);
                    arena.release();

                    if(!prod)
                    {
                        if(++total_spp >= max_spp)
                            break;
                    }
                    else
                        max_spp = 1000000;

                } while(sampler.next_sample());
            }
        }
    }

public:

    ~IsolatedPathTracer()
    {
        executer_.reset();
    }

    void initialize(const IsolatedPathTracerParams &params)
    {
        AGZ_HIERARCHY_TRY

        min_depth_ = params.min_depth;
        if(min_depth_ < 1)
            throw ObjectConstructionException("invalid min depth value: " + std::to_string(min_depth_));

        max_depth_ = params.max_depth;
        if(max_depth_ < min_depth_)
            throw ObjectConstructionException("invalid max depth value: " + std::to_string(max_depth_));

        cont_prob_ = params.cont_prob;
        if(cont_prob_ < 0 || cont_prob_ > 1)
            throw ObjectConstructionException("invalid continue prob value: " + std::to_string(cont_prob_));

        shading_aa_ = params.shading_aa;
        if(shading_aa_ <= 0)
            throw ObjectConstructionException("invalid shading_aa value: " + std::to_string(shading_aa_));

        background_aa_ = params.background_aa;
        if(background_aa_ <= 0)
            throw ObjectConstructionException("invalid background_aa value: " + std::to_string(background_aa_));

        background_color_ = params.background_color;
        hide_background_entity_ = params.hide_background_entity;

        worker_count_ = params.worker_count;
        
        task_grid_size_ = params.task_grid_size;
        if(task_grid_size_ <= 0)
            throw ObjectConstructionException("invalid task grid size value: " + std::to_string(task_grid_size_));

        sampler_prototype_ = params.sampler_prototype;

        AGZ_HIERARCHY_WRAP("in initializing isolated path tracer")
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

        auto func = [p_this = this,
                     &sampler = *sampler_prototype_,
                     &scene = scene,
                     &reporter = reporter,
                     film = film,
                     res = res]
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

        executer_->run_async(worker_count_, std::move(tasks), func);
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

std::shared_ptr<Renderer> create_isolated_path_tracer(
    const IsolatedPathTracerParams &params)
{
    auto ret = std::make_shared<IsolatedPathTracer>();
    ret->initialize(params);
    return ret;
}

AGZ_TRACER_END
