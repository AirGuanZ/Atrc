#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/medium.h>
#include <agz/tracer/core/scattering.h>
#include <agz/tracer/core/scene.h>

#include "../medium/medium_void.h"

AGZ_TRACER_BEGIN

class DefaultScene : public Scene
{
    const Camera *camera_ = nullptr;

    std::vector<const Light*> lights_;
    const Entity *env_lht_ent_ = nullptr;

    std::vector<const Entity*> entities_;
    AABB world_bound_;

    math::distribution::alias_sampler_t<real, size_t> light_selector_;
    std::vector<real> light_pdf_table_;

    void construct_light_sampler()
    {
        light_selector_.destroy();
        light_pdf_table_.clear();

        if(lights_.empty())
            return;

        light_pdf_table_.resize(lights_.size());
        for(size_t i = 0; i < lights_.size(); ++i)
            light_pdf_table_[i] = lights_[i]->power().lum();

        real sum = std::accumulate(light_pdf_table_.begin(), light_pdf_table_.end(), real(0));
        real ratio = 1 / sum;
        for(auto &p : light_pdf_table_)
            p *= ratio;

        light_selector_.initialize(light_pdf_table_.data(), light_pdf_table_.size());
    }

public:

    using Scene::Scene;

    static std::string description()
    {
        return R"___(
default [Scene]
    see doc of SceneBuilder
)___";
    }

    void set_camera(const Camera *camera) override
    {
        camera_ = camera;
        world_bound_ |= camera_->world_bound();
    }

    const Camera *camera() const noexcept override
    {
        return camera_;
    }

    void add_entity(const Entity *entity) override
    {
        assert(entity);
        entities_.push_back(entity);
        world_bound_ |= entity->world_bound();
        if(auto light = entity->as_light())
            lights_.push_back(light);
    }

    void set_env_light(const Entity *env_light) override
    {
        assert(env_light);
        env_lht_ent_ = env_light;
        if(auto light = env_light->as_light())
            lights_.push_back(light);
        else
            throw ObjectConstructionException("env_light must be a light source");
    }

    size_t light_count() const noexcept override
    {
        return lights_.size();
    }

    const Light *light(size_t idx) const noexcept override
    {
        return lights_[idx];
    }

    misc::span<const Light* const> lights() const noexcept override
    {
        auto ptr = lights_.data();
        return misc::span<const Light* const>(ptr, lights_.size());
    }

    SceneSampleLightResult sample_light(const Sample1 &sam) const noexcept override
    {
        SceneSampleLightResult ret = { nullptr, 0 };
        if(!light_selector_.available())
            return ret;

        size_t idx = light_selector_.sample(sam.u);
        assert(0 <= idx && idx < lights_.size());

        ret.light = lights_[idx];
        ret.pdf = light_pdf_table_[idx];
        return ret;
    }

    real light_pdf(const Light *light) const noexcept override
    {
        for(size_t i = 0; i < lights_.size(); ++i)
        {
            if(lights_[i] == light)
                return light_pdf_table_[i];
        }
        return 0;
    }

    bool has_intersection(const Ray &r) const noexcept override
    {
        for(auto ent : entities_)
        {
            if(ent->has_intersection(r))
                return true;
        }
        return env_lht_ent_ && env_lht_ent_->has_intersection(r);
    }

    bool closest_intersection(const Ray &r, EntityIntersection *inct) const noexcept override
    {
        assert(inct);

        real t_ray = std::numeric_limits<real>::infinity();

        EntityIntersection tmp;
        for(auto ent : entities_)
        {
            if(ent->closest_intersection(r, &tmp) && tmp.t < t_ray)
            {
                *inct = tmp;
                t_ray = tmp.t;
            }
        }

        if(!std::isinf(t_ray))
            return true;

        return env_lht_ent_ && env_lht_ent_->closest_intersection(r, inct);
    }

    bool next_scattering_point(const Ray &r, SampleScatteringResult *result, const Sample1 &sam, Arena &arena) const override
    {
        if(result->p_has_inct)
            *result->p_has_inct = false;

        if(r.t_max <= r.t_min)
            return false;

        EntityIntersection ent_inct;
        const Medium *medium;
        if(!closest_intersection(r, &ent_inct))
        {
            medium = &VoidMedium::VOID_MEDIUM();
            auto med_sample = medium->sample(r.o, r.d, r.t_min, r.t_max, sam);
            if(med_sample.invalid() || med_sample.inct.invalid())
                return false;
            result->pdf = med_sample.pdf;
            result->pnt = ScatteringPoint(med_sample.inct, arena);
            return true;
        }

        if(result->p_has_inct)
        {
            assert(result->p_inct);
            *result->p_has_inct = true;
            *result->p_inct = ent_inct;
        }

        if(ent_inct.t <= r.t_min)
        {
            result->pdf = 1;
            result->pnt = ScatteringPoint(ent_inct, arena);
            return true;
        }

        medium = ent_inct.wr_medium();
        auto med_sample = medium->sample(r.o, r.d, r.t_min, ent_inct.t, sam);
        if(med_sample.invalid())
            return false;

        if(med_sample.inct.invalid())
        {
            result->pdf = med_sample.pdf;
            result->pnt = ScatteringPoint(ent_inct, arena);
        }
        else
        {
            result->pdf = med_sample.pdf;
            result->pnt = ScatteringPoint(med_sample.inct, arena);
        }

        return true;
    }

    const Medium *determine_medium(const Vec3 &o, const Vec3 &d) const noexcept override
    {
        EntityIntersection inct;
        Ray r(o, d);
        if(closest_intersection(r, &inct))
            return inct.wr_medium();
        return &VoidMedium::VOID_MEDIUM();
    }

    AABB world_bound() const noexcept override
    {
        return world_bound_;
    }

    void start_rendering() override
    {
        construct_light_sampler();
    }
};

AGZT_IMPLEMENTATION(Scene, DefaultScene, "default")

AGZ_TRACER_END
