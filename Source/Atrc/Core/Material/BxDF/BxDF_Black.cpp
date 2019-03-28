#include <Atrc/Core/Material/BxDF/BxDF_Black.h>

namespace Atrc
{

BxDF_Black::BxDF_Black() noexcept
    : BxDF(BSDF_NULL)
{

}

Spectrum BxDF_Black::GetBaseColor() const noexcept
{
    return Spectrum();
}

Spectrum BxDF_Black::EvalUncolored(
    [[maybe_unused]] const Vec3 &wi, [[maybe_unused]] const Vec3 &wo,
    [[maybe_unused]] bool star) const noexcept
{
    return Spectrum();
}

std::optional<BxDF::SampleWiResult> BxDF_Black::SampleWi(
    [[maybe_unused]] const Vec3 &wo, [[maybe_unused]] bool star,
    [[maybe_unused]] const Vec3 &sample) const noexcept
{
    return std::nullopt;
}

Real BxDF_Black::SampleWiPDF(
    [[maybe_unused]] const Vec3 &wi, [[maybe_unused]] const Vec3 &wo,
    [[maybe_unused]] bool star) const noexcept
{
    return 0;
}

} // namespace Atrc
