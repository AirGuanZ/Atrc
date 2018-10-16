#include <Atrc/Material/Diffuse.h>
#include <Atrc/Math/Math.h>

AGZ_NS_BEG(Atrc)

namespace
{
    class DiffuseBxDF
        : ATRC_IMPLEMENTS BxDF,
        ATRC_PROPERTY AGZ::Uncopiable
    {
        CoordSys localCoord_;
        Spectrum color_;

    public:

        explicit DiffuseBxDF(const Intersection &inct, const Spectrum &color);

        Spectrum Eval(const Vec3r &wi, const Vec3r &wo) const override;

        Real PDF(const Vec3r &wo, const Vec3r &sample) const override;

        Option<BxDFSample> Sample(const Vec3r &wo) const override;
    };

    DiffuseBxDF::DiffuseBxDF(const Intersection &inct, const Spectrum &color)
        : localCoord_(CoordSys::FromZ(inct.nor)), color_(color)
    {

    }

    Spectrum DiffuseBxDF::Eval(const Vec3r &wi, const Vec3r &wo) const
    {
        if(Dot(wi, localCoord_.ez) > 0.0 && Dot(wo, localCoord_.ez) > 0.0)
            return color_;
        return SPECTRUM::BLACK;
    }

    Real DiffuseBxDF::PDF(const Vec3r &wo, const Vec3r &sample) const
    {
        return Max(CommonSampler::ZWeighted_OnUnitHemisphere::PDF(localCoord_.W2C(sample)), 0.0);
    }

    Option<BxDFSample> DiffuseBxDF::Sample(const Vec3r &wo) const
    {
        if(Dot(wo, localCoord_.ez) > 0.0)
        {
            auto [dir, pdf] = CommonSampler::ZWeighted_OnUnitHemisphere::Sample();
            return BxDFSample{ localCoord_.C2W(dir), color_, pdf };
        }
        return None;
    }
}

DiffuseMaterial::DiffuseMaterial(const Spectrum &color)
    : color_(color / PI<float>)
{

}

RC<BxDF> DiffuseMaterial::GetBxDF(const Intersection &inct, const Vec2r &matParam) const
{
    return NewRC<DiffuseBxDF>(inct, color_);
}

AGZ_NS_END(Atrc)
