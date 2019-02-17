#include <Atrc/Lib/Material/BxDF/BxDF_SpecularReflection.h>

namespace Atrc
{
    
BxDF_SpecularReflection::BxDF_SpecularReflection(const Fresnel *fresnel, const Spectrum &rc) noexcept
    : BxDF(BSDFType(BSDF_REFLECTION | BSDF_SPECULAR)), fresnel_(fresnel), rc_(rc)
{
    AGZ_ASSERT(fresnel);
}

Spectrum BxDF_SpecularReflection::GetAlbedo() const noexcept
{
    return rc_;
}

Spectrum BxDF_SpecularReflection::Eval(
    [[maybe_unused]] const CoordSystem &geoInShd, [[maybe_unused]] const Vec3& wi, [[maybe_unused]] const Vec3& wo,
    [[maybe_unused]] bool star) const noexcept
{
    return Spectrum();
}

std::optional<BxDF::SampleWiResult> BxDF_SpecularReflection::SampleWi(
    const CoordSystem &geoInShd, const Vec3 &wo, [[maybe_unused]] bool star, [[maybe_unused]] const Vec3 &sample) const noexcept
{
    if(wo.z <= 0 || !geoInShd.InPositiveHemisphere(wo))
        return std::nullopt;

    Vec3 nWo = wo.Normalize();
    SampleWiResult ret;
    ret.wi      = Vec3(0, 0, 2 * nWo.z) - nWo;
    ret.pdf     = 1;
    ret.coef    = fresnel_->Eval(nWo.z) * rc_ / nWo.z;
    ret.type    = type_;
    ret.isDelta = true;

    return ret;
}

Real BxDF_SpecularReflection::SampleWiPDF(
    [[maybe_unused]] const CoordSystem &geoInShd, [[maybe_unused]] const Vec3 &wi, [[maybe_unused]] const Vec3 &wo,
    [[maybe_unused]] bool star) const noexcept
{
    return 0;
}

} // namespace Atrc
