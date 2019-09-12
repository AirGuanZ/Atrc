#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture.h>
#include <agz/tracer/utility/reflection.h>

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
            
            Vec3 nwi = wi.normalize(), nwo = wo.normalize();
            auto opt_wit = refl_aux::refract(nwi, shading_coord_.z, eta_out_ / eta_in_);
            auto opt_wot = refl_aux::refract(nwo, shading_coord_.z, eta_out_ / eta_in_);
            if(!opt_wit || !opt_wot)
                return {};

            real Fr_i = refl_aux::dielectric_fresnel(eta_in_, eta_out_, nwi.z);
            real Fr_o = refl_aux::dielectric_fresnel(eta_in_, eta_out_, nwo.z);
            auto ret = color_ * color_ * (1 - Fr_i) * (1 - Fr_o) * internal_->eval(-*opt_wit, -*opt_wot, mode)
                * std::abs(cos(shading_coord_.z, -*opt_wit) / nwi.z);
            ret *= local_angle::normal_corr_factor(geometry_coord_, shading_coord_, wi);
            return ret;
        }

        BSDFSampleResult sample(const Vec3 &wo, TransportMode mode, const Sample3 &sam) const noexcept override
        {
            if(cause_black_fringes(wo))
                return BSDF_SAMPLE_RESULT_INVALID;

            Vec3 lwo = shading_coord_.global_to_local(wo).normalize();
            if(lwo.z <= 0)
                return BSDF_SAMPLE_RESULT_INVALID;

            real Fr = refl_aux::dielectric_fresnel(eta_in_, eta_out_, local_angle::cos_theta(lwo));
            if(sam.u < Fr)
            {
                Vec3 lwi = refl_aux::reflect(lwo, Vec3(0, 0, 1));
                Vec3 wi = shading_coord_.local_to_global(lwi);
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
            Vec3 wot = *opt_wot;
            if(cause_black_fringes(wot))
                return BSDF_SAMPLE_RESULT_INVALID;

            real new_sam_u = (sam.u - Fr) / (1 - Fr);
            auto internal_sample = internal_->sample(-*opt_wot, mode, { new_sam_u, sam.v, sam.w });
            if(!internal_sample.f)
                return BSDF_SAMPLE_RESULT_INVALID;
            internal_sample.f *= std::abs(cos(shading_coord_.z, internal_sample.dir));

            Vec3 internal_wi = -internal_sample.dir.normalize();
            auto opt_wi = refl_aux::refract(internal_wi, -shading_coord_.z, eta_in_ / eta_out_);
            if(!opt_wi)
                return BSDF_SAMPLE_RESULT_INVALID;
            
            Vec3 wi = *opt_wi;
            if(cause_black_fringes(wi))
                return BSDF_SAMPLE_RESULT_INVALID;

            real Fr_o = refl_aux::dielectric_fresnel(eta_in_, eta_out_, -internal_wi.z);
            
            BSDFSampleResult ret;
            ret.dir      = wi;
            ret.f        = color_ * color_ * (1 - Fr) * (1 - Fr_o) * internal_sample.f / std::abs(cos(wi, shading_coord_.z));
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

            Vec3 nwi = wi.normalize(), nwo = wo.normalize();
            auto opt_wit = refl_aux::refract(nwi, shading_coord_.z, eta_out_ / eta_in_);
            auto opt_wot = refl_aux::refract(nwo, shading_coord_.z, eta_out_ / eta_in_);
            if(!opt_wit || !opt_wot)
                return 0;

            Vec3 lwo = shading_coord_.global_to_local(wo).normalize();
            real Fr = refl_aux::dielectric_fresnel(eta_in_, eta_out_, local_angle::cos_theta(lwo));

            return (1 - Fr) * internal_->pdf(-*opt_wit, -*opt_wot, mode);
        }

        Spectrum albedo() const noexcept override
        {
            return color_ * internal_->albedo();
        }

        bool is_black() const noexcept override
        {
            return false;
        }
    };

} // namespace anonymous

class MirrorVarnish : public Material
{
    const Material *internal_ = nullptr;
    const Texture *eta_in_    = nullptr;
    const Texture *eta_out_   = nullptr;
    const Texture *color_     = nullptr;

public:

    static std::string description()
    {
        return R"___(
mirror_varnish [Material]
    internal [Material] internal material
    eta_in   [Texture]  internal IOR map
    eta_out  [Texture]  external IOR map
    color    [Texture]  varnish color map
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        init_customed_flag(params);

        internal_ = MaterialFactory.create(params.child_group("internal"), init_ctx);
        eta_in_   = TextureFactory.create(params.child_group("eta_in"), init_ctx);
        eta_out_  = TextureFactory.create(params.child_group("eta_out"), init_ctx);
        color_    = TextureFactory.create(params.child_group("color"), init_ctx);

        AGZ_HIERARCHY_WRAP("in initializing mirror varnish material")
    }

    ShadingPoint shade(const EntityIntersection &inct, Arena &arena) const override
    {
        auto internal_shd = internal_->shade(inct, arena);
        
        real eta_in    = eta_in_->sample_real(inct.uv);
        real eta_out   = eta_out_->sample_real(inct.uv);
        Spectrum color = color_->sample_spectrum(inct.uv);
        auto bsdf = arena.create<MirrorVarnishBSDF>(
            inct.geometry_coord, inct.user_coord, internal_shd.bsdf, eta_in, eta_out, color);
        
        return { bsdf };
    }
};

AGZT_IMPLEMENTATION(Material, MirrorVarnish, "mirror_varnish")

AGZ_TRACER_END
