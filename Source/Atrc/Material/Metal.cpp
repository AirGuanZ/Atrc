#include <Atrc/Material/Metal.h>
#include <Atrc/Math/Math.h>

AGZ_NS_BEG(Atrc)

namespace
{
    class MetalBxDF
        : ATRC_IMPLEMENTS BxDF,
          ATRC_PROPERTY AGZ::Uncopiable
    {
        Vec3r nor_;
        Spectrum color_;
        Real roughness_;

    public:

        explicit MetalBxDF(const Intersection &inct, const Spectrum &color, Real roughness);

        Spectrum Eval(const Vec3r &wi, const Vec3r &wo) const override;

        Option<BxDFSample> Sample(const Vec3r &wi) const override;

        bool IsSpecular() const override;
    };

    MetalBxDF::MetalBxDF(const Intersection &inct, const Spectrum &color, Real roughness)
        : nor_(inct.nor), color_(color), roughness_(roughness)
    {

    }

    Spectrum MetalBxDF::Eval(const Vec3r &wi, const Vec3r &wo) const
    {
        return SPECTRUM::BLACK;
    }

    Option<BxDFSample> MetalBxDF::Sample(const Vec3r &wi) const
    {
        if(Dot(wi, nor_) > 0.0)
        {
            Vec3r dir = 2.0 * Dot(nor_, wi) * nor_ - wi;
            if(roughness_)
                dir = (dir + roughness_ * CommonSampler::Uniform_InUnitSphere::Sample().sample).Normalize();
            Real cos = Dot(dir, nor_);
            if(cos <= 0.0)
                return None;
            return BxDFSample{ dir, color_ / SS(cos), 1.0 };
        }
        return None;
    }

    bool MetalBxDF::IsSpecular() const
    {
        return true;
    }
}

MetalMaterial::MetalMaterial(const Spectrum &color, Real roughness)
    : color_(color), roughness_(roughness)
{
    
}

RC<BxDF> MetalMaterial::GetBxDF(const Intersection &inct, const Vec2r &matParam) const
{
    return NewRC<MetalBxDF>(inct, color_, roughness_);
}

AGZ_NS_END(Atrc)
