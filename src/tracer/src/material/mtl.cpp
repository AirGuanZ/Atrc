#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture.h>

#include "./bsdf_aggregate.h"
#include "./microfacet.h"

AGZ_TRACER_BEGIN

namespace mtl_impl
{
    
    class DiffuseComponent : public InternalBSDF
    {
        const Spectrum albedo_;

    public:

        explicit DiffuseComponent(const Spectrum &albedo) noexcept
            : albedo_(albedo)
        {
            
        }

        Spectrum eval(const Vec3 &in_dir, const Vec3 &out_dir, TransportMode) const noexcept override
        {
            if(in_dir.z <= 0 || out_dir.z <= 0)
                return Spectrum();
            return albedo_ / PI_r;
        }

        real proj_wi_factor(const Vec3 &wi) const noexcept override
        {
            return std::abs(wi.z);
        }

        BSDFSampleResult sample(const Vec3 &out_dir, TransportMode transport_mode, const Sample3 &sam) const noexcept override
        {
            if(out_dir.z <= 0)
                return BSDF_SAMPLE_RESULT_INVALID;

            auto [local_in, pdf] = math::distribution::zweighted_on_hemisphere(sam.v, sam.w);
            if(pdf < EPS)
                return BSDF_SAMPLE_RESULT_INVALID;

            BSDFSampleResult ret;
            ret.dir            = local_in;
            ret.f              = albedo_ / PI_r;
            ret.pdf            = pdf;
            ret.mode           = transport_mode;
            ret.is_delta       = false;

            return ret;
        }

        real pdf(const Vec3 &in_dir, const Vec3 &out_dir, TransportMode) const noexcept override
        {
            if(in_dir.z <= 0 || out_dir.z <= 0)
                return 0;
            return math::distribution::zweighted_on_hemisphere_pdf(in_dir.z);
        }

        Spectrum albedo() const noexcept override
        {
            return albedo_;
        }
    };

    class SpecularComponent : public InternalBSDF
    {
        const Spectrum color_;
        const real ns_;

        Vec3 pow_cos_on_hemisphere(real e, const Sample2 &sam) const noexcept
        {
            real cos_theta_h = std::pow(sam.u, 1 / (e + 1));
            real sin_theta_h = local_angle::cos_2_sin(cos_theta_h);
            real phi = 2 * PI_r * sam.v;

            return Vec3(sin_theta_h * std::cos(phi), sin_theta_h * std::sin(phi), cos_theta_h).normalize();
        }

        real pow_cos_on_hemisphere_pdf(real e, real cos_theta) const noexcept
        {
            return (e + 1) / (2 * PI_r) * std::pow(cos_theta, e);
        }

    public:

        SpecularComponent(const Spectrum &color, real ns) noexcept
            : color_(color), ns_(ns)
        {
            
        }

        Spectrum eval(const Vec3 &in_dir, const Vec3 &out_dir, TransportMode) const noexcept override
        {
            if(in_dir.z <= 0 || out_dir.z <= 0)
                return Spectrum();

            Vec3 wh = (in_dir + out_dir).normalize();
            return color_ * Spectrum(std::pow(wh.z, ns_));
        }

        real proj_wi_factor(const Vec3 &wi) const noexcept override
        {
            return std::abs(wi.z);
        }

        BSDFSampleResult sample(const Vec3 &out_dir, TransportMode transport_mode, const Sample3& sam) const noexcept override
        {
            if(out_dir.z <= 0)
                return BSDF_SAMPLE_RESULT_INVALID;

            auto wh = pow_cos_on_hemisphere(ns_, { sam.u, sam.v });
            Vec3 wi = 2 * dot(out_dir, wh) * wh - out_dir;
            if(wi.z <= 0)
                return BSDF_SAMPLE_RESULT_INVALID;
            
            real pdf = pow_cos_on_hemisphere_pdf(ns_, wh.z) / (4 * dot(out_dir, wh));

            BSDFSampleResult ret;
            ret.f              = color_ * std::pow(wh.z, ns_);
            ret.pdf            = pdf;
            ret.dir            = wi.normalize();
            ret.mode           = transport_mode;
            ret.is_delta       = false;
            
            return ret;
        }

        real pdf(const Vec3 &in_dir, const Vec3 &out_dir, TransportMode) const noexcept override
        {
            if(in_dir.z <= 0 || out_dir.z <= 0)
                return 0;

            Vec3 wh = (in_dir + out_dir).normalize();
            return pow_cos_on_hemisphere_pdf(ns_, wh.z) / (4 * dot(out_dir, wh));
        }

        Spectrum albedo() const noexcept override
        {
            return color_;
        }
    };

} // namespace mtl_impl

class MTL : public Material
{
    const Texture *kd_ = nullptr;
    const Texture *ks_ = nullptr;
    const Texture *ns_ = nullptr;

public:

    using Material::Material;

    static std::string description()
    {
        return R"___(
mtl [Material]
    kd [Texture] kd map
    ks [Texture] ks map
    ns [Texture] ns map

    see http://paulbourke.net/dataformats/mtl/
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        kd_ = TextureFactory.create(params.child_group("kd"), init_ctx);
        ks_ = TextureFactory.create(params.child_group("ks"), init_ctx);
        ns_ = TextureFactory.create(params.child_group("ns"), init_ctx);
    
        AGZ_HIERARCHY_WRAP("in initializing mtl material object")
    }

    ShadingPoint shade(const EntityIntersection &inct, Arena &arena) const override
    {
        Spectrum kd = kd_->sample_spectrum(inct.uv);
        Spectrum ks = ks_->sample_spectrum(inct.uv);
        real ns     = ns_->sample_real(inct.uv);

        auto bsdf     = arena.create<BSDFAggregate<2>>(inct.geometry_coord, inct.user_coord);
        auto diffuse  = arena.create<mtl_impl::DiffuseComponent>(kd);
        auto specular = arena.create<mtl_impl::SpecularComponent>(ks, ns);
        bsdf->add(diffuse);
        bsdf->add(specular);
        
        return { bsdf };
    }
};

AGZT_IMPLEMENTATION(Material, MTL, "mtl")

AGZ_TRACER_END
