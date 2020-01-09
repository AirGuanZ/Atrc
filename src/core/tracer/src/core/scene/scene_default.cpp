#include <memory>
#include <unordered_map>
#include <vector>

#include <agz/tracer/core/aggregate.h>
#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/medium.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/factory/raw/medium.h>
#include <agz/tracer/factory/raw/scene.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

class DefaultScene : public Scene
{
    std::shared_ptr<Medium> void_medium_;

    std::shared_ptr<const Camera> camera_;

    std::vector<Light*>                      lights_;
    std::vector<EnvirLight *>                envir_lights_;
    std::vector<std::shared_ptr<EnvirLight>> owner_envir_lights_;

    std::vector<std::shared_ptr<Entity>> entities_;

    std::shared_ptr<const Aggregate> aggregate_;
    
    math::distribution::alias_sampler_t<real, size_t> light_selector_;
    std::vector<real> light_pdf_table_;
    std::unordered_map<const Light*, real> light_ptr_to_pdf_;

    void construct_light_sampler()
    {
        light_selector_.destroy();
        light_pdf_table_.clear();

        if(lights_.empty())
            return;

        light_pdf_table_.resize(lights_.size());
        for(size_t i = 0; i < lights_.size(); ++i)
            light_pdf_table_[i] = lights_[i]->power().lum();

        const real sum = std::accumulate(light_pdf_table_.begin(), light_pdf_table_.end(), real(0));
        const real ratio = 1 / sum;
        for(auto &p : light_pdf_table_)
            p *= ratio;

        light_selector_.initialize(light_pdf_table_.data(), light_pdf_table_.size());

        light_ptr_to_pdf_.clear();
        for(size_t i = 0; i < light_pdf_table_.size(); ++i)
        {
            const Light *light = lights_[i];
            const real pdf = light_pdf_table_[i];
            light_ptr_to_pdf_.insert(std::make_pair(light, pdf));
        }
    }

public:

    explicit DefaultScene(const DefaultSceneParams &params)
    {
        void_medium_ = create_void();

        owner_envir_lights_ = params.envir_lights;
        for(auto &nonarea_light : params.envir_lights)
        {
            lights_.push_back(nonarea_light.get());
            envir_lights_.push_back(nonarea_light.get());
        }

        if(envir_lights_.size() > 1)
            throw ObjectConstructionException("more than 1 environment lights");

        std::vector<std::shared_ptr<const Entity>> const_entities;
        const_entities.reserve(params.entities.size());
        for(auto &ent : params.entities)
        {
            if(auto light = ent->as_light())
                lights_.push_back(light);
            const_entities.push_back(ent);
        }
        aggregate_ = params.aggregate;
        params.aggregate->build(const_entities);

        entities_ = params.entities;
    }

    void set_camera(std::shared_ptr<const Camera> camera) override
    {
        camera_ = camera;
    }

    const Camera *get_camera() const noexcept override
    {
        return camera_.get();
    }

    misc::span<const Light* const> lights() const noexcept override
    {
        const auto ptr = lights_.data();
        return misc::span<const Light* const>(ptr, lights_.size());
    }

    misc::span<const EnvirLight * const> envir_lights() const noexcept override
    {
        const auto ptr = envir_lights_.data();
        return misc::span<const EnvirLight * const>(ptr, envir_lights_.size());
    }

    SceneSampleLightResult sample_light(const Sample1 &sam) const noexcept override
    {
        SceneSampleLightResult ret = { nullptr, 0 };
        if(!light_selector_.available())
            return ret;

        const size_t idx = light_selector_.sample(sam.u);
        assert(0 <= idx && idx < lights_.size());

        ret.light = lights_[idx];
        ret.pdf = light_pdf_table_[idx];
        return ret;
    }

    real light_pdf(const AreaLight *light) const noexcept override
    {
        const auto it = light_ptr_to_pdf_.find(light);
        return it != light_ptr_to_pdf_.end() ? it->second : real(0);
    }

    bool has_intersection(const Ray &r) const noexcept override
    {
        return aggregate_->has_intersection(r);
    }

    bool visible(const Vec3 &A, const Vec3 &B) const noexcept override
    {
        const real dis = (A - B).length();
        const Ray shadow_ray(A, (B - A).normalize(), EPS, dis - EPS);
        return !has_intersection(shadow_ray);
    }

    bool closest_intersection(const Ray &r, EntityIntersection *inct) const noexcept override
    {
        return aggregate_->closest_intersection(r, inct);
    }

    void start_rendering() override
    {
        AABB world_bound;
        for(auto &ent : entities_)
            world_bound |= ent->world_bound();

        for(auto light : lights_)
            light->preprocess(world_bound);

        construct_light_sampler();
    }
};

std::shared_ptr<Scene> create_default_scene(const DefaultSceneParams &params)
{
    return std::make_shared<DefaultScene>(params);
}

AGZ_TRACER_END
