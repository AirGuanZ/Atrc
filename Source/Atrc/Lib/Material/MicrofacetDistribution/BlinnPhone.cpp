#include <Atrc/Lib/Material/MicrofacetDistribution/BlinnPhong.h>

namespace Atrc
{

BlinnPhong::BlinnPhong(Real e) noexcept
    : e_(e)
{
    
}
    
Real BlinnPhong::D(const Vec3 &H) const noexcept
{
    return H.z <= 0 ? Real(0) : (e_ + 2) / (2 * PI) * Pow(H.z, e_);
}

Real BlinnPhong::G(const Vec3 &H, const Vec3 &wi, const Vec3 &wo) const noexcept
{
    Real NdH  = H.z;
    Real NdWi = wi.z;
    Real NdWo = wo.z;
    Real WodH = Dot(wo, H);
    return Min(Real(1),
               Min(2 * Abs(NdH * NdWo / WodH),
                   2 * Abs(NdH * NdWi / WodH)));
}

std::optional<BlinnPhong::SampleWiResult> BlinnPhong::SampleWi(const CoordSystem &geoInShd, const Vec3 &wo, const Vec2 &sample) const noexcept
{
    if(wo.z <= 0 || !geoInShd.InPositiveHemisphere(wo))
        return std::nullopt;

    Real cosTheta = Pow(sample.x, 1 / (e_ + 1));
    Real sinTheta = Sqrt(Max(Real(0), 1 - cosTheta * cosTheta));
    Real phi = 2 * PI * sample.y;

    Vec3 H(sinTheta * Cos(phi), sinTheta * Sin(phi), cosTheta);
    if(H.z <= 0)
        return std::nullopt;

    SampleWiResult ret;
    ret.wi = 2 * Dot(wo, H) * H - wo;
    if(ret.wi.z <= 0 || !geoInShd.InPositiveHemisphere(ret.wi))
        return std::nullopt;
    ret.pdf = (e_ + 1) * Pow(cosTheta, e_) / (2 * PI * 4 * Dot(wo, H));

    return ret;
}

Real BlinnPhong::SampleWiPDF([[maybe_unused]] const CoordSystem &geoInShd, const Vec3 &wi, const Vec3 &wo) const noexcept
{
    Vec3 H = (wi + wo).Normalize();
    Real cosTheta = H.z;

    return (e_ + 1) * Pow(cosTheta, e_) / (2 * PI * 4 * Dot(wo, H));
}

} // namespace Atrc
