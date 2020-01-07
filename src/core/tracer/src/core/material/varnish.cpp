#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture2d.h>
#include <agz/tracer/utility/reflection.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

namespace
{
    
    class MirrorVarnishBSDF : public LocalBSDF
    {
        const BSDF *internal_;
        real eta_in_;
        real eta_out_;
        Spectrum color_;

    public:

        MirrorVarnishBSDF(const Coord &geometry_coord, const Coord &shading_coord,
                          const BSDF *internal, real eta_in, real eta_out, const Spectrum &color) noexcept
            : LocalBSDF(geometry_coord, shading_coord),
              internal_(internal), eta_in_(eta_in), eta_out_(eta_out), color_(color)
        {
            
        }

        Spectrum eval(const Vec3 &wi, const Vec3 &wo, TransportMode mode) const noexcept override
        {
            if(cause_black_fringes(wi) || cause_black_fringes(wo))
                return {};
            
            const Vec3 nwi = wi.normalize(), nwo = wo.normalize();
            const auto opt_wot = refl_aux::refract(nwo, shading_coord_.z, eta_out_ / eta_in_);
            if(!opt_wot)
                return {};

            const real Fr_o = refl_aux::dielectric_fresnel(eta_in_, eta_out_, std::abs(cos(nwo, shading_coord_.z)));
            Spectrum ret = color_ * color_ * (1 - Fr_o) * internal_->eval(nwi, -*opt_wot, mode);
            ret *= local_angle::normal_corr_factor(geometry_coord_, shading_coord_, wi);
            return ret;
        }

        BSDFSampleResult sample(const Vec3 &wo, TransportMode mode, const Sample3 &sam) const noexcept override
        {
            if(cause_black_fringes(wo))
                return BSDF_SAMPLE_RESULT_INVALID;

            const Vec3 lwo = shading_coord_.global_to_local(wo).normalize();
            if(lwo.z <= 0)
                return BSDF_SAMPLE_RESULT_INVALID;

            const real Fr = refl_aux::dielectric_fresnel(eta_in_, eta_out_, local_angle::cos_theta(lwo));
            if(sam.u < Fr)
            {
                const Vec3 lwi = refl_aux::reflect(lwo, Vec3(0, 0, 1));
                const Vec3 wi = shading_coord_.local_to_global(lwi);
                if(cause_black_fringes(wi))
                    return BSDF_SAMPLE_RESULT_INVALID;

                BSDFSampleResult ret;
                ret.dir      = wi;
                ret.f        = color_ * Fr / std::abs(lwi.z);
                ret.pdf      = Fr;
                ret.is_delta = true;
                ret.mode     = mode;

                ret.f *= local_angle::normal_corr_factor(geometry_coord_, shading_coord_, ret.dir);
                return ret;
            }

            auto opt_wot = refl_aux::refract(wo.normalize(), shading_coord_.z, eta_out_ / eta_in_);
            if(!opt_wot)
                return BSDF_SAMPLE_RESULT_INVALID;
            const Vec3 wot = *opt_wot;
            if(cause_black_fringes(wot))
                return BSDF_SAMPLE_RESULT_INVALID;

            const real new_sam_u = (sam.u - Fr) / (1 - Fr);
            auto internal_sample = internal_->sample(-wot, mode, { new_sam_u, sam.v, sam.w });
            if(!internal_sample.f)
                return BSDF_SAMPLE_RESULT_INVALID;
            
            const Vec3 wi = internal_sample.dir;
            if(cause_black_fringes(wi))
                return BSDF_SAMPLE_RESULT_INVALID;
            
            BSDFSampleResult ret;
            ret.dir      = wi;
            ret.f        = color_ * color_ * (1 - Fr) * internal_sample.f;
            ret.pdf      = (1 - Fr) * internal_sample.pdf;
            ret.is_delta = internal_sample.is_delta;
            ret.mode     = mode;

            ret.f *= local_angle::normal_corr_factor(geometry_coord_, shading_coord_, ret.dir);
            return ret;
        }

        real pdf(const Vec3 &wi, const Vec3 &wo, TransportMode mode) const noexcept override
        {
            if(cause_black_fringes(wi) || cause_black_fringes(wo))
                return 0;

            const Vec3 nwi = wi.normalize(), nwo = wo.normalize();
            auto opt_wit = refl_aux::refract(nwi, shading_coord_.z, eta_out_ / eta_in_);
            auto opt_wot = refl_aux::refract(nwo, shading_coord_.z, eta_out_ / eta_in_);
            if(!opt_wit || !opt_wot)
                return 0;

            const Vec3 lwo = shading_coord_.global_to_local(wo).normalize();
            const real Fr = refl_aux::dielectric_fresnel(eta_in_, eta_out_, local_angle::cos_theta(lwo));

            return (1 - Fr) * internal_->pdf(-*opt_wit, -*opt_wot, mode);
        }

        Spectrum albedo() const noexcept override
        {
            return color_ * internal_->albedo();
        }

        bool is_delta() const noexcept override
        {
            return true;
        }
    };

} // namespace anonymous

class MirrorVarnish : public Material
{
    std::shared_ptr<const Material> internal_;
    std::shared_ptr<const Texture2D> eta_in_;
    std::shared_ptr<const Texture2D> eta_out_;
    std::shared_ptr<const Texture2D> color_;

public:

    MirrorVarnish(
        std::shared_ptr<const Material> internal,
        std::shared_ptr<const Texture2D> eta_in,
        std::shared_ptr<const Texture2D> eta_out,
        std::shared_ptr<const Texture2D> color)
    {
        internal_ = std::move(internal);
        eta_in_   = std::move(eta_in);
        eta_out_  = std::move(eta_out);
        color_    = std::move(color);
    }

    ShadingPoint shade(const EntityIntersection &inct, Arena &arena) const override
    {
        const ShadingPoint internal_shd = internal_->shade(inct, arena);
        
        const real eta_in    = eta_in_->sample_real(inct.uv);
        const real eta_out   = eta_out_->sample_real(inct.uv);
        const Spectrum color = color_->sample_spectrum(inct.uv);
        const BSDF *bsdf = arena.create<MirrorVarnishBSDF>(
            inct.geometry_coord, inct.user_coord, internal_shd.bsdf, eta_in, eta_out, color);
        
        return { bsdf, inct.user_coord.z };
    }
};

std::shared_ptr<Material> create_mirror_varnish(
    std::shared_ptr<const Material> internal,
    std::shared_ptr<const Texture2D> eta_in,
    std::shared_ptr<const Texture2D> eta_out,
    std::shared_ptr<const Texture2D> color)
{
    return std::make_shared<MirrorVarnish>(
        std::move(internal), std::move(eta_in), std::move(eta_out), std::move(color));
}

AGZ_TRACER_END
