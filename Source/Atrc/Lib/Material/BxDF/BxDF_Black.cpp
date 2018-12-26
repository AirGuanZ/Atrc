#include <Atrc/Lib/Material/BxDF/BxDF_Black.h>

namespace Atrc
{

BxDF_Black::BxDF_Black() noexcept
    : BxDF(BSDF_NULL)
{

}

Spectrum BxDF_Black::GetAlbedo() const noexcept
{
    return Spectrum();
}

Spectrum BxDF_Black::Eval(
    [[maybe_unused]] const CoordSystem &geoInShd,
    [[maybe_unused]] const Vec3 &wi, [[maybe_unused]] const Vec3 &wo,
    [[maybe_unused]] bool star) const noexcept
{
    return Spectrum();
}

Option<BxDF::SampleWiResult> BxDF_Black::SampleWi(
    [[maybe_unused]] const CoordSystem &geoInShd,
    [[maybe_unused]] const Vec3 &wo, [[maybe_unused]] bool star,
    [[maybe_unused]] const Vec2 &sample) const noexcept
{
    return None;
}

Real BxDF_Black::SampleWiPDF(
    [[maybe_unused]] const CoordSystem &geoInShd,
    [[maybe_unused]] const Vec3 &wi, [[maybe_unused]] const Vec3 &wo,
    [[maybe_unused]] bool star) const noexcept
{
    return 0;
}

} // namespace Atrc
