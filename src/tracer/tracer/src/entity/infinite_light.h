#pragma once

#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/infinite_light.h>
#include <agz/tracer/core/scene.h>

#include "../material/ideal_black.h"
#include "./medium_interface.h"

AGZ_TRACER_BEGIN

template<typename Impl>
class InfiniteLightImplToLight : public Light
{
    static_assert(std::is_base_of_v<InfiniteLightImpl, Impl>);
    InfiniteLightCore core_;
    Impl *impl_ = nullptr;

    MediumInterface medium_interface_;

    template<typename>
    friend class InfiniteLightEntity;

public:

    using Light::Light;

    static std::string description()
    {
        return Impl::description();
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        impl_ = init_ctx.arena->create<Impl>();
        impl_->initialize(params, init_ctx);
        medium_interface_.initialize(params, init_ctx);
    }

    LightSampleResult sample(const Vec3 &ref, const Sample5 &sam) const noexcept override
    {
        if(!core_.in_scene(ref))
            return LIGHT_SAMPLE_RESULT_NULL;
        return impl_->sample(core_, ref, { sam.u, sam.v, sam.w, sam.r });
    }

    Spectrum power() const noexcept override
    {
        return impl_->power(core_);
    }

    void preprocess(const Scene &scene) override
    {
        core_.preprocess(scene);
    }

    Spectrum radiance(const SurfacePoint &, const Vec3 &light_to_out) const noexcept override
    {
        return impl_->radiance(core_, -light_to_out);
    }

    real pdf(const Vec3 &ref, const SurfacePoint &spt) const noexcept override
    {
        if(!core_.in_scene(ref))
            return 0;
        Vec3 ref_to_light = spt.pos - ref;
        return impl_->pdf(core_, ref, ref_to_light);
    }

    LightEmitResult emit(const Sample5 &sam) const noexcept override
    {
        auto ret = impl_->emit(core_, { sam.u, sam.v, sam.w, sam.r });
        ret.medium = medium_interface_.out;
        return ret;
    }

    void emit_pdf(const SurfacePoint &spt, const Vec3 &light_to_out, real *pdf_pos, real *pdf_dir) const noexcept override
    {
        return impl_->emit_pdf(core_, spt, light_to_out, pdf_pos, pdf_dir);
    }
};

class InfiniteLightImplAggregate : public Light
{
    InfiniteLightCore core_;

    std::vector<InfiniteLightImpl*> impls_;
    std::vector<real> impl_probs_;
    math::distribution::alias_sampler_t<real> impl_sampler_;

    MediumInterface medium_interface_;

    template<typename>
    friend class InfiniteLightEntity;

    void prepare_sampler()
    {
        assert(!impls_.empty());
        if(!impl_probs_.empty())
            return;

        real total_power = 0;
        impl_probs_.reserve(impls_.size());
        for(auto impl : impls_)
        {
            real power = impl->power(core_).lum();
            total_power += power;
            impl_probs_.push_back(power);
        }

        if(total_power <= EPS)
        {
            real prob = real(1) / impls_.size();
            for(auto &p : impl_probs_)
                p = prob;
        }
        else
        {
            real ratio = 1 / total_power;
            for(auto &p : impl_probs_)
                p *= ratio;
        }

        impl_sampler_.initialize(impl_probs_.data(), static_cast<int>(impl_probs_.size()));
    }

public:

    using Light::Light;

    static std::string description()
    {
        return R"___(
aggregate [(Nonarea)Light]
    impls [[(Nonarea)Light]] infinite light implementations
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        auto &arr = params.child_array("impls");
        
        impls_.reserve(arr.size());
        for(size_t i = 0; i < arr.size(); ++i)
        {
            auto &group = arr.at_group(i);
            if(stdstr::ends_with(group.child_str("type"), "//"))
                continue;
            auto impl = InfiniteLightImplFactory.create(group, init_ctx);
            impls_.push_back(impl);
        }

        if(impls_.empty())
            throw ObjectConstructionException("infinite light implementation array is empty");

        medium_interface_.initialize(params, init_ctx);

        AGZ_HIERARCHY_WRAP("in initializing infinite light aggregate");
    }

    LightSampleResult sample(const Vec3 &ref, const Sample5 &sam) const noexcept override
    {
        if(!core_.in_scene(ref))
            return LIGHT_SAMPLE_RESULT_NULL;

        size_t impl_idx = impl_sampler_.sample(sam.u);
        real impl_prob = impl_probs_[impl_idx];

        LightSampleResult ret = impls_[impl_idx]->sample(core_, ref, { sam.v, sam.w, sam.r, sam.s });
        ret.pdf *= impl_prob;
        Vec3 ref_to_light = (ret.spt.pos - ref).normalize();
        for(size_t i = 0; i < impls_.size(); ++i)
        {
            if(i == impl_idx)
                continue;
            ret.radiance += impls_[i]->radiance(core_, ref_to_light);
            ret.pdf += impl_probs_[i] * impls_[i]->pdf(core_, ref, ref_to_light);
        }

        return ret;
    }

    Spectrum power() const noexcept override
    {
        Spectrum ret;
        for(auto impl : impls_)
            ret += impl->power(core_);
        return ret;
    }

    void preprocess(const Scene &scene) override
    {
        core_.preprocess(scene);
        prepare_sampler();
    }

    Spectrum radiance(const SurfacePoint &spt, const Vec3 &light_to_ref) const noexcept override
    {
        Vec3 ref_to_light = -light_to_ref;
        Spectrum ret;
        for(auto impl : impls_)
            ret += impl->radiance(core_, ref_to_light);
        return ret;
    }

    real pdf(const Vec3 &ref, const SurfacePoint &spt) const noexcept override
    {
        if(!core_.in_scene(ref))
            return 0;

        real ret = 0;
        Vec3 ref_to_light = (spt.pos - ref).normalize();
        for(size_t i = 0; i < impls_.size(); ++i)
            ret += impl_probs_[i] * impls_[i]->pdf(core_, ref, ref_to_light);

        return ret;
    }

    LightEmitResult emit(const Sample5 &sam) const noexcept override
    {
        size_t impl_idx = impl_sampler_.sample(sam.u);
        real impl_prob = impl_probs_[impl_idx];

        LightEmitResult ret = impls_[impl_idx]->emit(core_, { sam.v, sam.w, sam.r, sam.s });
        ret.pdf_pos *= impl_prob;
        ret.pdf_dir *= impl_prob;
        for(size_t i = 0; i < impls_.size(); ++i)
        {
            if(i == impl_idx)
                continue;
            real apdf_pos, apdf_dir;
            impls_[i]->emit_pdf(core_, ret.spt, ret.dir, &apdf_pos, &apdf_dir);
            ret.pdf_pos += impl_probs_[i] * apdf_pos;
            ret.pdf_dir += impl_probs_[i] * apdf_dir;
            ret.radiance += impls_[i]->radiance(core_, ret.dir);
        }

        ret.medium = medium_interface_.out;

        return ret;
    }

    void emit_pdf(const SurfacePoint &spt, const Vec3 &light_to_out, real *pdf_pos, real *pdf_dir) const noexcept override
    {
        real pos = 0, dir = 0;
        for(size_t i = 0; i < impls_.size(); ++i)
        {
            real tpos, tdir;
            impls_[i]->emit_pdf(core_, spt, light_to_out, &tpos, &tdir);
            pos += impl_probs_[i] * tpos;
            dir += impl_probs_[i] * tdir;
        }
        *pdf_pos = pos;
        *pdf_dir = dir;
    }
};

template<typename TLight>
class InfiniteLightEntity : public Entity
{
    static_assert(std::is_base_of_v<Light, TLight>);

    TLight *light_ = nullptr;

public:

    using Entity::Entity;

    static std::string description()
    {
        return TLight::description();
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        light_ = init_ctx.arena->create<TLight>();
        light_->initialize(params, init_ctx);
    }

    bool has_intersection(const Ray &r) const noexcept override
    {
        return light_->core_.has_intersection(r);
    }

    bool closest_intersection(const Ray &r, EntityIntersection *inct) const noexcept override
    {
        if(!light_->core_.intersection(r, inct))
            return false;
        inct->entity     = this;
        inct->material   = &IdealBlack::IDEAL_BLACK_INSTANCE();
        inct->medium_in  = light_->medium_interface_.in;
        inct->medium_out = light_->medium_interface_.out;
        return true;
    }

    AABB world_bound() const noexcept override
    {
        return AABB();
    }

    const Light *as_light() const noexcept override
    {
        return light_;
    }

    Light *as_light() noexcept override
    {
        return light_;
    }
};

AGZ_TRACER_END
