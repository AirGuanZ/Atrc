#include <Atrc/Core/Material/BxDF/BxDF_Diffuse.h>

/*
    NOTE: wo.z <= 0时，diffuse bxdf的eval结果应该是0，但在存在法线偏移时，wo有可能在
    几何坐标系的正面，而在着色坐标系的反面，此时会形成黑色色块，因此我手动移除了这一限制条件。
    这固然是物理不正确的，但在无法线偏移时不会造成任何影响，何况法线偏移本来就是不物理的。
*/

namespace Atrc
{

BxDF_Diffuse::BxDF_Diffuse(const Spectrum &albedo) noexcept
    : BxDF(BSDF_NONSPECULAR), albedo_(albedo)
{

}

Spectrum BxDF_Diffuse::GetBaseColor() const noexcept
{
    return albedo_;
}

Spectrum BxDF_Diffuse::EvalUncolored(const Vec3 &wi, const Vec3 &wo, [[maybe_unused]] bool star) const noexcept
{
    if(wi.z <= 0 || wo.z <= 0)
        return Spectrum();
    return Spectrum(1 / PI);
}

std::optional<BxDF::SampleWiResult> BxDF_Diffuse::SampleWi(const Vec3 &wo, [[maybe_unused]] bool star, const Vec3 &sample) const noexcept
{
    /*if(wo.z <= 0)
        return std::nullopt;*/

    auto [sam, pdf] = AGZ::Math::DistributionTransform
        ::ZWeightedOnUnitHemisphere<Real>::Transform(sample.xy());

    if(sam.z <= 0 || !pdf)
        return std::nullopt;

    SampleWiResult ret;
    ret.coef    = albedo_ / PI;
    ret.pdf     = pdf;
    ret.type    = type_;
    ret.wi      = sam;
    ret.isDelta = false;
    return ret;
}

Real BxDF_Diffuse::SampleWiPDF(const Vec3 &wi, const Vec3 &wo, [[maybe_unused]] bool star) const noexcept
{
    if(wi.z <= 0 || wo.z <= 0)
        return 0;
    return AGZ::Math::DistributionTransform
        ::ZWeightedOnUnitHemisphere<Real>::PDF(wi);
}

} // namespace Atrc
