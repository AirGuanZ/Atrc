#include <Atrc/Lib/Material/BxDF/BxDF_Diffuse.h>

namespace Atrc
{

BxDF_Diffuse::BxDF_Diffuse(const Spectrum &albedo) noexcept
    : BxDF(BSDF_DIFFUSE), albedo_(albedo)
{

}

Spectrum BxDF_Diffuse::GetAlbedo() const noexcept
{
    return albedo_;
}

Spectrum BxDF_Diffuse::Eval(const CoordSystem &geoInShd, const Vec3 &wi, const Vec3 &wo) const noexcept
{
    if(wi.z <= 0 || wo.z <= 0 || geoInShd.World2Local(wi).z <= 0 || geoInShd.World2Local(wo).z <= 0)
        return Spectrum();
    return albedo_ / PI;
}

Option<BxDF::SampleWiResult> BxDF_Diffuse::SampleWi(const CoordSystem &geoInShd, const Vec3 &wo, const Vec2 &sample) const noexcept
{
    if(wo.z <= 0 || geoInShd.World2Local(wo).z <= 0)
        return None;

    auto [sam, pdf] = AGZ::Math::DistributionTransform
        ::ZWeightedOnUnitHemisphere<Real>::Transform(sample);
    if(geoInShd.World2Local(sam).z <= 0 || !pdf)
        return None;

    SampleWiResult ret;
    ret.coef    = albedo_ / PI;
    ret.pdf     = pdf;
    ret.type    = type_;
    ret.wi      = sam;
    ret.isDelta = false;
    return ret;
}

Real BxDF_Diffuse::SampleWiPDF(const CoordSystem &geoInShd, const Vec3 &wi, const Vec3 &wo) const noexcept
{
    if(wi.z <= 0 || wo.z <= 0 || geoInShd.World2Local(wi).z <= 0 || geoInShd.World2Local(wo).z <= 0)
        return 0;
    return AGZ::Math::DistributionTransform
        ::ZWeightedOnUnitHemisphere<Real>::PDF(wi.Normalize());
}

} // namespace Atrc
