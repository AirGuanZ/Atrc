#include <Atrc/Lib/Material/BxDF/BxDF_Void.h>

namespace Atrc
{

BxDF_Void::BxDF_Void() noexcept
    : BxDF(BSDFType(BSDF_TRANSMISSION | BSDF_SPECULAR))
{

}

Spectrum BxDF_Void::GetAlbedo() const noexcept
{
    return Spectrum();
}

Spectrum BxDF_Void::Eval(
    [[maybe_unused]] const CoordSystem &geoInShd,
    [[maybe_unused]] const Vec3 &wi, [[maybe_unused]] const Vec3 &wo) const noexcept
{
    return Spectrum();
}

Option<BxDF::SampleWiResult> BxDF_Void::SampleWi(
    [[maybe_unused]] const CoordSystem &geoInShd,
    const Vec3 &wo, [[maybe_unused]] const Vec2 &sample) const noexcept
{
    SampleWiResult ret;
    ret.coef = Spectrum(Real(1));
    ret.wi   = -wo;
    ret.pdf  = 1;
    ret.type = type_;
    ret.isDelta = true;
    return ret;
}

Real BxDF_Void::SampleWiPDF(
    [[maybe_unused]] const CoordSystem &geoInShd,
    [[maybe_unused]] const Vec3 &wi, [[maybe_unused]] const Vec3 &wo) const noexcept
{
    return 0;
}

} // namespace Atrc
