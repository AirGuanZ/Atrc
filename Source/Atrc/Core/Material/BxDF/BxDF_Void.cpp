#include <Atrc/Core/Material/BxDF/BxDF_Void.h>

namespace Atrc
{

BxDF_Void::BxDF_Void() noexcept
    : BxDF(BSDFType(BSDF_TRANSMISSION | BSDF_SPECULAR))
{

}

Spectrum BxDF_Void::GetBaseColor() const noexcept
{
    return Spectrum();
}

Spectrum BxDF_Void::EvalUncolored(
    [[maybe_unused]] const Vec3 &wi, [[maybe_unused]] const Vec3 &wo,
    [[maybe_unused]] bool star) const noexcept
{
    return Spectrum();
}

std::optional<BxDF::SampleWiResult> BxDF_Void::SampleWi(
    const Vec3 &wo, [[maybe_unused]] bool star,
    [[maybe_unused]] const Vec3 &sample) const noexcept
{
    if(!wo.z)
        return std::nullopt;

    SampleWiResult ret;
    ret.coef = Spectrum(Real(1)) / Abs(wo.z);
    ret.wi   = -wo;
    ret.pdf  = 1;
    ret.type = type_;
    ret.isDelta = true;
    return ret;
}

Real BxDF_Void::SampleWiPDF(
    [[maybe_unused]] const Vec3 &wi, [[maybe_unused]] const Vec3 &wo,
    [[maybe_unused]] bool star) const noexcept
{
    return 0;
}

} // namespace Atrc
