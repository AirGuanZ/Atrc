#include <Atrc/Lib/Material/BxDF/BxDF_DisneyReflection.h>
#include <Atrc/Lib/Material/Utility/Microfacet.h>

namespace Atrc
{

BxDF_DisneyReflection::BxDF_DisneyReflection(
    const Spectrum &baseColor,
    Real metallic,
    Real subsurface,
    Real specular,
    Real specularTint,
    Real roughness,
    Real anisotropic,
    Real sheen,
    Real sheenTint,
    Real clearcoat,
    Real clearcoatGloss) noexcept
    : BxDF(BSDFType(BSDF_REFLECTION | BSDF_NONSPECULAR)),
      baseColor_(baseColor),
      metallic_(Saturate(metallic)),
      subsurface_(Saturate(subsurface)),
      specular_(Saturate(specular)),
      specularTint_(Saturate(specularTint)),
      roughness_(Clamp(roughness, Real(0.03), Real(1))),
      anisotropic_(Saturate(anisotropic)),
      sheen_(Saturate(sheen)),
      sheenTint_(Saturate(sheenTint)),
      clearcoat_(Saturate(clearcoat)),
      clearcoatGloss_(Saturate(clearcoatGloss))
{
    Real aspect = anisotropic > 0 ? Sqrt(1 - Real(0.9) * anisotropic) : Real(1);
    ax_ = Sqr(roughness_) / aspect;
    ay_ = Sqr(roughness_) * aspect;

    clearcoatRoughness_ = Mix(Real(0.1), Real(0.01), clearcoatGloss_);

    sampleWeights_.wd = Min(Real(0.8), 1 - metallic_);
    sampleWeights_.ws = (1 - sampleWeights_.wd) * 2 / (2 + clearcoat_);
    sampleWeights_.wc = (1 - sampleWeights_.wd) * clearcoat_ / (2 + clearcoat_);
}

namespace
{
    Real f_diffuse(Real FI, Real FO, Real cosThetaD2, Real roughness) noexcept
    {
        Real FD90 = Real(0.5) + 2 * cosThetaD2 * roughness;
        return (1 + (FD90 - 1) * FI) * (1 + (FD90 - 1) * FO);
    }

    Real f_subsurface(Real cosThetaI, Real cosThetaO, Real FI, Real FO, Real cosThetaD2, Real roughness) noexcept
    {
        Real Fss90 = cosThetaD2 * roughness;
        Real Fss = (1 + (Fss90 - 1) * FI) * (1 + (Fss90 - 1) * FO);
        return Real(1.25) * (Fss * (1 / (cosThetaI + cosThetaO) - Real(0.5)) + Real(0.5));
    }

} // namespace anonymous

Spectrum BxDF_DisneyReflection::Eval(const Vec3 &wi, const Vec3 &wo, bool star) const noexcept
{
    if(wi.z <= 0 || wo.z <= 0)
        return Spectrum();

    Vec3 wh = (wi + wo).Normalize();

    Real cosThetaD = Dot(wi, wh), cosThetaD2 = Sqr(cosThetaD);
    
    Real cosThetaH = CosTheta(wh), sinThetaH = Cos2Sin(cosThetaH);
    Real phiH = Phi(wh), cosPhiH = Cos(phiH), sinPhiH = Cos2Sin(cosPhiH);

    Real cosThetaI = CosTheta(wi), sinThetaI = Cos2Sin(cosThetaI), tanThetaI = sinThetaI / cosThetaI;
    Real cosThetaO = CosTheta(wo), sinThetaO = Cos2Sin(cosThetaO), tanThetaO = sinThetaO / cosThetaO;

    Real phiI = Phi(wi), cosPhiI = Cos(phiI), sinPhiI = Cos2Sin(cosPhiI);
    Real phiO = Phi(wo), cosPhiO = Cos(phiO), sinPhiO = Cos2Sin(cosPhiO);
    
    Real FI = Microfacet::OneMinus5(cosThetaI), FO = Microfacet::OneMinus5(cosThetaO), FD = Microfacet::OneMinus5(cosThetaD);

    Real lum = Luminance(baseColor_);
    if(lum < EPS)
        return Spectrum();
    Spectrum Ctint = baseColor_ / lum;

    // diffuse, subsurface and sheen

    Real f_d = 0, f_ss = 0;
    Spectrum f_sh;

    if(metallic_ < 1)
    {
        if(subsurface_ < 1)
            f_d = f_diffuse(FI, FO, cosThetaD2, roughness_);
        if(subsurface_ > 0)
            f_ss = f_subsurface(cosThetaI, cosThetaO, FI, FO, cosThetaD2, roughness_);
        if(sheen_ > 0)
            f_sh = Mix(Spectrum(1), Ctint, sheenTint_) * sheen_ * FD;
    }

    // specular

    Spectrum Cs = Mix(Real(0.08) * specular_ * Mix(Spectrum(1), Ctint, specularTint_), baseColor_, metallic_);
    Spectrum Fs = Cs + (Spectrum(1) - Cs) * FD;

    Real Gs = Microfacet::SmithAnisotropicGTR2(cosPhiI, sinPhiI, ax_, ay_, tanThetaI)
            * Microfacet::SmithAnisotropicGTR2(cosPhiO, sinPhiO, ax_, ay_, tanThetaO);
    //Real Gs = Microfacet::SmithGTR2(tanThetaI, roughness_ * roughness_)
    //        * Microfacet::SmithGTR2(tanThetaO, roughness_ * roughness_);

    Real Ds = Microfacet::AnisotropicGTR2(sinPhiH, cosPhiH, sinThetaH, cosThetaH, ax_, ay_);
    //Real Ds = Microfacet::GTR2(cosThetaH, Sqr(roughness_));

    // clearcoat

    Real Fc = 0, Gc = 0, Dc = 0;
    if(clearcoat_ > 0)
    {
        Fc = Real(0.04) + Real(0.96) * FD;
        Gc = Microfacet::SmithGTR2(tanThetaI, Real(0.25))
           * Microfacet::SmithGTR2(tanThetaO, Real(0.25));
        Dc = Microfacet::GTR1(sinThetaH, cosThetaH, clearcoatRoughness_);
    }

    Real microfacetDem = Real(1) / 4 / (cosThetaI * cosThetaO);

    return (1 - metallic_) * (baseColor_ / PI * Mix(f_d, f_ss, subsurface_) + f_sh)
          + Fs * Gs * Ds * microfacetDem
          + clearcoat_ / 4 * Fc * Gc * Dc * microfacetDem;
}

std::optional<BxDF::SampleWiResult> BxDF_DisneyReflection::SampleWi(const Vec3 &wo, bool star, const Vec3 &sample) const noexcept
{
    if(wo.z <= 0)
        return std::nullopt;

    Vec3 wi;

    if(sample.x < sampleWeights_.wd)
    {
        // sample diffuse
        wi = AGZ::Math::DistributionTransform::ZWeightedOnUnitHemisphere<Real>::Transform(sample.yz()).sample;
        wi = wi.Normalize();
    }
    else if(sample.x - sampleWeights_.wd < sampleWeights_.ws)
    {
        // sample specular
        
        Vec3 wh = Microfacet::SampleAnisotropicGTR2(ax_, ay_, sample.yz());
        //Vec3 wh = Microfacet::SampleGTR2(Sqr(roughness_), sample.yz());
        if(wh.z <= 0)
            return std::nullopt;

        wi = (2 * Dot(wo, wh) * wh - wo).Normalize();
        if(wi.z <= 0 || Dot(wi, wh) <= 0)
            return std::nullopt;
    }
    else
    {
        // sample clearcoat

        Vec3 wh = Microfacet::SampleGTR1(clearcoatRoughness_, sample.yz());
        if(wh.z <= 0)
            return std::nullopt;

        wi = (2 * Dot(wo, wh) * wh - wo).Normalize();
        if(wi.z <= 0 || Dot(wi, wh) <= 0)
            return std::nullopt;
    }

    Real pdf = SampleWiPDF(wi, wo, star);
    Spectrum coef = Eval(wi, wo, star);
    if(pdf < EPS || coef.HasInf())
        return std::nullopt;

    SampleWiResult ret;
    ret.wi      = wi;
    ret.coef    = Eval(wi, wo, star);
    ret.isDelta = false;
    ret.pdf     = pdf;
    ret.type    = type_;

    return ret;
}

Real BxDF_DisneyReflection::SampleWiPDF(const Vec3 &wi, const Vec3 &wo, bool star) const noexcept
{
    if(wi.z <= 0 || wo.z <= 0)
        return 0;

    Vec3 wh = (wi + wo).Normalize();
    Real cosThetaH = CosTheta(wh), sinThetaH = Cos2Sin(cosThetaH);
    Real phiH = Phi(wh), cosPhiH = Cos(phiH), sinPhiH = Cos2Sin(cosPhiH);
    Real cosThetaD = Dot(wi, wh);

    Real diffusePDF = AGZ::Math::DistributionTransform::ZWeightedOnUnitHemisphere<Real>::PDF(wi);
    Real specularPDF = cosThetaH * Microfacet::AnisotropicGTR2(sinPhiH, cosPhiH, sinThetaH, cosThetaH, ax_, ay_) / (4 * cosThetaD);
    Real clearcoatPDF = cosThetaH * Microfacet::GTR1(sinThetaH, cosThetaH, clearcoatRoughness_) / (4 * cosThetaD);

    return sampleWeights_.wd * diffusePDF + sampleWeights_.ws * specularPDF + sampleWeights_.wc * clearcoatPDF;
}

} // namespace Atrc
