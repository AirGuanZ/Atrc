#include <Atrc/Lib/Material/BxDF/BxDF_Diffuse.h>

/*
    NOTE: wo.z <= 0时，diffuse bxdf的eval结果应该是0，但在存在法线偏移时，wo有可能在
    几何坐标系的正面，而在着色坐标系的反面，此时会形成黑色色块，因此我手动移除了这一限制条件。
    这固然是物理不正确的，但在无法线偏移时不会造成任何影响，何况法线偏移本来就是不物理的。
*/

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
    if(wi.z <= 0 || /*wo.z <= 0 ||*/ !geoInShd.InPositiveHemisphere(wi) || !geoInShd.InPositiveHemisphere(wo))
        return Spectrum();
    return albedo_ / PI;
}

Option<BxDF::SampleWiResult> BxDF_Diffuse::SampleWi(const CoordSystem &geoInShd, const Vec3 &wo, const Vec2 &sample) const noexcept
{
    if(!geoInShd.InPositiveHemisphere(wo))
        return None;

    auto [sam, pdf] = AGZ::Math::DistributionTransform
        ::ZWeightedOnUnitHemisphere<Real>::Transform(sample);

    /*
        1. 如果sam在几何坐标系背面，那么这一采样的射线一定会被物体拦截，不如直接返回None好了
        2. pdf在理论上不可能为0，但因为数值原因在MSVC上使用float时有微小的概率为0，因此将其滤除
    */
    if(!geoInShd.InPositiveHemisphere(sam) || !pdf)
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
    if(wi.z <= 0 || /*wo.z <= 0 || */!geoInShd.InPositiveHemisphere(wi) || !geoInShd.InPositiveHemisphere(wo))
        return 0;
    return AGZ::Math::DistributionTransform
        ::ZWeightedOnUnitHemisphere<Real>::PDF(wi.Normalize());
}

} // namespace Atrc
