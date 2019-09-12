#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture.h>

#include "./utility/microfacet.h"

AGZ_TRACER_BEGIN

namespace mtl_impl
{

    // IMPROVE: handle black fringes
    template<int MAX_BSDF_CNT>
    class BSDFAggregate : public LocalBSDF
    {
        const BSDF *bsdfs_[MAX_BSDF_CNT];
        int bsdf_count_;

    public:

        BSDFAggregate(const Coord &geometry_coord, const Coord &shading_coord) noexcept
            : LocalBSDF(geometry_coord, shading_coord),
            bsdfs_{ nullptr }, bsdf_count_(0)
        {

        }

        void add(const BSDF *bsdf)
        {
            assert(bsdf_count_ < MAX_BSDF_CNT);
            bsdfs_[bsdf_count_++] = bsdf;
        }

        Spectrum eval(const Vec3 &in_dir, const Vec3 &out_dir, TransportMode transport_mode) const noexcept override
        {
            assert(bsdf_count_ > 0);

            Vec3 local_in = shading_coord_.global_to_local(in_dir).normalize();
            Vec3 local_out = shading_coord_.global_to_local(out_dir).normalize();

            Spectrum ret;
            for(int i = 0; i < bsdf_count_; ++i)
                ret += bsdfs_[i]->eval(local_in, local_out, transport_mode);

            ret *= local_angle::normal_corr_factor(geometry_coord_, shading_coord_, in_dir);
            return ret;
        }

        BSDFSampleResult sample(const Vec3 &out_dir, TransportMode transport_mode, const Sample3 &sam) const noexcept override
        {
            assert(bsdf_count_ > 0);

            auto[bsdf_idx, new_sam_u] = math::distribution::extract_uniform_int(sam.u, 0, bsdf_count_);
            auto bsdf = bsdfs_[bsdf_idx];

            Vec3 local_out = shading_coord_.global_to_local(out_dir).normalize();
            auto ret = bsdf->sample(local_out, transport_mode, { new_sam_u, sam.v, sam.w });

            if(!ret.dir)
                return BSDF_SAMPLE_RESULT_INVALID;

            ret.dir = ret.dir.normalize();

            if(ret.is_delta)
            {
                ret.pdf /= bsdf_count_;
                ret.dir = shading_coord_.local_to_global(ret.dir).normalize();
                ret.f *= local_angle::normal_corr_factor(geometry_coord_, shading_coord_, ret.dir);
                return ret;
            }

            for(int i = 0; i < bsdf_count_; ++i)
            {
                if(i == bsdf_idx)
                    continue;
                ret.f += bsdfs_[i]->eval(ret.dir, local_out, transport_mode);
                ret.pdf += bsdfs_[i]->pdf(ret.dir, local_out, transport_mode);
            }
            ret.pdf /= bsdf_count_;
            ret.dir = shading_coord_.local_to_global(ret.dir).normalize();

            ret.f *= local_angle::normal_corr_factor(geometry_coord_, shading_coord_, ret.dir);
            return ret;
        }

        real pdf(const Vec3 &in_dir, const Vec3 &out_dir, TransportMode transport_mode) const noexcept override
        {
            assert(bsdf_count_ > 0);

            Vec3 local_in = shading_coord_.global_to_local(in_dir).normalize();
            Vec3 local_out = shading_coord_.global_to_local(out_dir).normalize();

            real ret = 0;
            for(int i = 0; i < bsdf_count_; ++i)
                ret += bsdfs_[i]->pdf(local_in, local_out, transport_mode);
            return ret / bsdf_count_;
        }

        Spectrum albedo() const noexcept override
        {
            Spectrum ret;
            for(int i = 0; i < bsdf_count_; ++i)
                ret += bsdfs_[i]->albedo();
            return ret;
        }

        bool is_black() const noexcept override
        {
            for(int i = 0; i < bsdf_count_; ++i)
            {
                if(!bsdfs_[i]->is_black())
                    return false;
            }
            return true;
        }
    };
    
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

        Vec3 sample_pow_cos_on_hemisphere(real e, const Sample2 &sam) const noexcept
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
            real D = (ns_ + 1) / (2 * PI_r) * std::pow(wh.z, ns_);
            return color_ * D / (4 * in_dir.z * out_dir.z);
        }

        BSDFSampleResult sample(const Vec3 &out_dir, TransportMode transport_mode, const Sample3& sam) const noexcept override
        {
            if(out_dir.z <= 0)
                return BSDF_SAMPLE_RESULT_INVALID;

            auto wh = sample_pow_cos_on_hemisphere(ns_, { sam.u, sam.v });
            Vec3 wi = (2 * dot(out_dir, wh) * wh - out_dir).normalize();
            if(wi.z <= 0)
                return BSDF_SAMPLE_RESULT_INVALID;

            real D = (ns_ + 1) / (2 * PI_r) * std::pow(wh.z, ns_);
            real pdf = D / (4 * dot(out_dir, wh));

            BSDFSampleResult ret;
            ret.f              = color_ * D / (4 * wi.z * out_dir.z);
            ret.pdf            = pdf;
            ret.dir            = wi;
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

        init_customed_flag(params);

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

        // ��֤�����غ�
        real dem = 1;
        for(int i = 0; i < 3; ++i)
            dem = (std::max)(kd[i] + ks[i], dem);
        kd /= dem;
        ks /= dem;

        auto bsdf     = arena.create<mtl_impl::BSDFAggregate<2>>(inct.geometry_coord, inct.user_coord);
        auto diffuse  = arena.create<mtl_impl::DiffuseComponent>(kd);
        auto specular = arena.create<mtl_impl::SpecularComponent>(ks, ns);
        bsdf->add(diffuse);
        bsdf->add(specular);
        
        return { bsdf };
    }
};

AGZT_IMPLEMENTATION(Material, MTL, "mtl")

AGZ_TRACER_END
