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

        Real maxPhi_;

    public:

        explicit MetalBxDF(const Intersection &inct, const Spectrum &color, Real roughness);

        Spectrum Eval(const Vec3r &wi, const Vec3r &wo) const override;

        Real PDF(const Vec3r &wo, const Vec3r &sample) const override;

        Option<BxDFSample> Sample(const Vec3r &wo) const override;

        bool IsSpecular() const override;
    };

    MetalBxDF::MetalBxDF(const Intersection &inct, const Spectrum &color, Real roughness)
        : nor_(inct.nor), color_(color), roughness_(roughness)
    {
        maxPhi_ = Arctan(roughness);
    }

    /*Spectrum MetalBxDF::Eval(const Vec3r &wi, const Vec3r &wo) const
    {
        return SPECTRUM::BLACK;
    }

    Real MetalBxDF::PDF(const Vec3r &wo, const Vec3r &sample) const
    {
        return 0.0;
    }

    Option<BxDFSample> MetalBxDF::Sample(const Vec3r &wo) const
    {
        if(Dot(wo, nor_) > 0.0)
        {
            Vec3r dir = 2.0 * Dot(nor_, wo) * nor_ - wo;
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
    }*/

    Spectrum MetalBxDF::Eval(const Vec3r &wi, const Vec3r &wo) const
    {
        if(!roughness_)
            return SPECTRUM::BLACK;

        Real dotWi = Dot(nor_, wi), dotWo = Dot(nor_, wo);
        if(dotWi <= 0.0 || dotWo <= 0.0)
            return SPECTRUM::BLACK;

        Vec3r idealWo = 2.0 * dotWi * nor_ - wi;
        Real cosWiWo = Dot(idealWo, wo);
        Real phi = Arccos(cosWiWo);

        if(phi >= maxPhi_)
            return SPECTRUM::BLACK;

        Real beta = Arcsin(Sin(phi) / roughness_);
        Real t = PI<Real> / 2 - beta - phi;
        Real z = Cos(PI<Real> / 2 - t);

        return color_ * SS(z / PI<Real> /*/ Dot(wi, nor_)*/);
    }

    Real MetalBxDF::PDF(const Vec3r &wi, const Vec3r &wo) const
    {
        if(!roughness_)
            return 0.0;

        /*Vec3r idealWo = 2.0 * Dot(wi, nor_) * nor_ - wi;
        Real phi = Arccos(Dot(idealWo, wo));
        if(phi >= maxPhi_)
            return 1.0;

        Real beta = Arcsin(Sin(phi) / roughness_);

        return Cos(beta + phi) / PI<Real>;*/
        return Dot(wi, nor_) / PI<Real>;
    }

    Option<BxDFSample> MetalBxDF::Sample(const Vec3r &wo) const
    {
        if(Dot(wo, nor_) <= 0.0)
            return None;

        if(!roughness_)
        {
            Vec3r dir = 2.0 * Dot(nor_, wo) * nor_ - wo;
            return BxDFSample{ dir, color_ / SS(Dot(dir, nor_)), 1.0 };
        }

        /*Vec3r idealWi = 2.0 * Dot(wo, nor_) * nor_ - wo;
        auto [sam, pdf] = CommonSampler::ZWeighted_OnUnitHemisphere::Sample();
        Vec3r wi = (idealWi + CoordSys::FromZ(idealWi).C2W(roughness_ * sam)).Normalize();

        if(Dot(wi, nor_) <= 0.0)
            return None;*/

        auto [wi, pdf] = CommonSampler::ZWeighted_OnUnitHemisphere::Sample();
        wi = CoordSys::FromZ(nor_).C2W(wi);

        Spectrum f = Eval(wi, wo);
        return BxDFSample{ wi, f, pdf };
    }

    bool MetalBxDF::IsSpecular() const
    {
        return !roughness_;
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
