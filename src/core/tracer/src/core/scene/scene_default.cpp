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
    std::shared_ptr<Medium> void_medium_ = create_void();

    std::shared_ptr<const Camera> camera_;

    std::vector<Light*> lights_;
    std::vector<NonareaLight*> nonarea_lights_;
    std::vector<std::shared_ptr<NonareaLight>> owner_nonarea_lights_;

    std::shared_ptr<const Aggregate> aggregate_;
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

    void initialize(const DefaultSceneParams &params)
    {
        owner_nonarea_lights_ = params.nonarea_lights_;
        for(auto &nonarea_light : params.nonarea_lights_)
        {
            lights_.push_back(nonarea_light.get());
            nonarea_lights_.push_back(nonarea_light.get());
        }

        aggregate_ = params.aggregate;
        std::vector<std::shared_ptr<const Entity>> const_entities;
        const_entities.reserve(params.entities.size());
        for(auto &ent : params.entities)
        {
            if(auto light = ent->as_light())
                lights_.push_back(light);
            const_entities.push_back(ent);
        }
        params.aggregate->build(const_entities);
    }

    void set_camera(std::shared_ptr<const Camera> camera) override
    {
        camera_ = camera;
    }

    const Camera *camera() const noexcept override
    {
        return camera_.get();
    }

    misc::span<const Light* const> lights() const noexcept override
    {
        auto ptr = lights_.data();
        return misc::span<const Light* const>(ptr, lights_.size());
    }

    misc::span<const NonareaLight* const> nonarea_lights() const noexcept override
    {
        auto ptr = nonarea_lights_.data();
        return misc::span<const NonareaLight* const>(ptr, nonarea_lights_.size());
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

    real light_pdf(const AreaLight *light) const noexcept override
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
        return aggregate_->has_intersection(r);
    }

    bool closest_intersection(const Ray &r, EntityIntersection *inct) const noexcept override
    {
        return aggregate_->closest_intersection(r, inct);
    }

    const Medium *determine_medium(const Vec3 &o, const Vec3 &d) const noexcept override
    {
        EntityIntersection inct;
        Ray r(o, d);
        if(closest_intersection(r, &inct))
            return inct.wr_medium();
        return void_medium_.get();
    }

    AABB world_bound() const noexcept override
    {
        return world_bound_;
    }

    void start_rendering() override
    {
        world_bound_ = AABB();
        world_bound_ |= aggregate_->world_bound();
        //world_bound_ |= camera_->get_world_bound();

        // 避免数值问题导致某些场景中的点不在world bound中

        Vec3 delta = world_bound_.high - world_bound_.low;
        world_bound_.low  -= real(0.02) * delta;
        world_bound_.high += real(0.02) * delta;

        for(auto light : lights_)
            light->preprocess(*this);

        construct_light_sampler();
    }
};

std::shared_ptr<Scene> create_default_scene(const DefaultSceneParams &params)
{
    auto ret = std::make_shared<DefaultScene>();
    ret->initialize(params);
    return ret;
}

AGZ_TRACER_END
