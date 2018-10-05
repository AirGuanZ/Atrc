#include <Atrc/Material/Diffuse.h>

AGZ_NS_BEG(Atrc)

DiffuseBxDF::DiffuseBxDF(const Intersection &inct, const Spectrum &color)
    : localCoord_(CoordSys::FromZ(inct.nor)), color_(color)
{

}

BxDFType DiffuseBxDF::GetType() const
{
    return BXDF_REFLECTION | BXDF_DIFFUSE;
}

Spectrum DiffuseBxDF::Eval(const Vec3r &wi, const Vec3r &wo) const
{
    if(Dot(wi, localCoord_.ez) > 0.0 && Dot(wo, localCoord_.ez) > 0.0)
        return color_;
    return SPECTRUM::BLACK;
}

Option<BxDFSample> DiffuseBxDF::Sample(const Vec3r &wi, BxDFType type) const
{
    if(type.Contains(BXDF_REFLECTION | BXDF_DIFFUSE) &&
        Dot(wi, localCoord_.ez) > 0.0)
    {
        auto[dir, pdf] = CommonSampler::ZWeighted_OnUnitHemisphere::Sample();
        return BxDFSample{ localCoord_.C2W(dir), color_, pdf };
    }
    return None;
}

DiffuseMaterial::DiffuseMaterial(const Spectrum &color)
    : color_(color)
{

}

RC<BxDF> DiffuseMaterial::GetBxDF(const Intersection &inct, const Vec2r &matParam) const
{
    return NewRC<DiffuseBxDF>(inct, color_);
}

AGZ_NS_END(Atrc)
