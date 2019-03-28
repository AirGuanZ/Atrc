#include <Atrc/Core/Material/BxDF/BxDF_DisneySpecular.h>
#include <Atrc/Core/Material/Utility/Microfacet.h>

namespace Atrc
{
    
BxDF_DisneySpecular::BxDF_DisneySpecular(
    const Spectrum &baseColor, Real specular, Real specularTint,
    Real metallic, Real roughness, Real anisotropic) noexcept
    : BxDF(BSDFType(BSDF_REFLECTION | BSDF_NONSPECULAR)),
      baseColor_(baseColor),
      specular_(specular), specularTint_(specularTint),
      metallic_(metallic), anisotropic_(anisotropic)
{
    roughness_ = Clamp(roughness, Real(0.05), Real(1));
    Real aspect = Sqrt(1 - Real(0.9) * anisotropic);
    ax_ = Sqr(roughness_) / aspect;
    ay_ = Sqr(roughness_) * aspect;
}

Spectrum BxDF_DisneySpecular::Eval(const Vec3 &wi, const Vec3 &wo, bool star) const noexcept
{
    if(wi.z <= 0 || wo.z <= 0)
        return Spectrum();
    
    Vec3 wh = (wi + wo).Normalize();
    Real cosThetaH = CosTheta(wh), sinThetaH = Cos2Sin(cosThetaH);
    Real phiH = Phi(wh), cosPhiH = Cos(phiH), sinPhiH = Cos2Sin(cosPhiH);

    Real cosThetaI = CosTheta(wi), sinThetaI = Cos2Sin(cosThetaI), tanThetaI = sinThetaI / cosThetaI;
    Real cosThetaO = CosTheta(wo), sinThetaO = Cos2Sin(cosThetaO), tanThetaO = sinThetaO / cosThetaO;

    Real phiI = Phi(wi), cosPhiI = Cos(phiI), sinPhiI = Cos2Sin(cosPhiI);
    Real phiO = Phi(wo), cosPhiO = Cos(phiO), sinPhiO = Cos2Sin(cosPhiO);

    Real cosThetaD = Dot(wi, wh);

    Real D = Microfacet::AnisotropicGTR2(sinPhiH, cosPhiH, sinThetaH, cosThetaH, ax_, ay_);
    Real G = Microfacet::SmithAnisotropicGTR2(cosPhiI, sinPhiI, ax_, ay_, tanThetaI)
           * Microfacet::SmithAnisotropicGTR2(cosPhiO, sinPhiO, ax_, ay_, tanThetaO);

    Real lum = Luminance(baseColor_);
    if(!lum)
        return Spectrum();

    Spectrum Ctint = baseColor_ / lum;
    Spectrum Cs = Mix(Real(0.08) * specular_ * Mix(Spectrum(1), Ctint, specularTint_), baseColor_, metallic_);
    Spectrum Fs = Cs + (Spectrum(1) - Cs) * Microfacet::OneMinus5(cosThetaD);

    return Fs * D * G / (4 * cosThetaI * cosThetaD);
}

std::optional<BxDF::SampleWiResult> BxDF_DisneySpecular::SampleWi(const Vec3 &wo, bool star, const Vec3 &sample) const noexcept
{
    if(wo.z <= 0)
        return std::nullopt;

    Vec3 wh = Microfacet::SampleAnisotropicGTR2(ax_, ay_, sample.xy());
    if(wh.z <= 0)
        return std::nullopt;

    Vec3 wi = (2 * Dot(wo, wh) * wh - wo).Normalize();
    if(wi.z <= 0 || Dot(wi, wh) <= 0)
        return std::nullopt;

    Real cosThetaH = CosTheta(wh), sinThetaH = Cos2Sin(cosThetaH);
    Real phiH = Phi(wh), cosPhiH = Cos(phiH), sinPhiH = Cos2Sin(cosPhiH);
    Real cosThetaD = Dot(wi, wh);

    SampleWiResult ret;
    ret.wi = wi;
    ret.coef = Eval(wi, wo, star);
    ret.isDelta = false;
    ret.pdf = cosThetaH * Microfacet::AnisotropicGTR2(sinPhiH, cosPhiH, sinThetaH, cosThetaH, ax_, ay_) / (4 * cosThetaD);
    ret.type = type_;

    if(ret.coef.HasInf() || ret.pdf < EPS)
        return std::nullopt;

    return ret;
}

Real BxDF_DisneySpecular::SampleWiPDF(const Vec3 &wi, const Vec3 &wo, bool star) const noexcept
{
    if(wi.z <= 0 || wo.z <= 0)
        return 0;

    Vec3 wh = (wi + wo).Normalize();
    Real cosThetaH = CosTheta(wh), sinThetaH = Cos2Sin(cosThetaH);
    Real phiH = Phi(wh), cosPhiH = Cos(phiH), sinPhiH = Cos2Sin(cosPhiH);
    Real cosThetaD = Dot(wi, wh);

    return cosThetaH * Microfacet::AnisotropicGTR2(sinPhiH, cosPhiH, sinThetaH, cosThetaH, ax_, ay_) / (4 * cosThetaD);
}

} // namespace Atrc
