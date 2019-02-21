#include <Atrc/Lib/Material/BxDF/BxDF_DisneyPrincipled.h>
/*
namespace Atrc
{

namespace
{
    Real GTR_D(Real cosThetaH, Real gamma, Real alpha)
    {
        if(cosThetaH <= 0)
            return 0;
        return 1 / Pow(1 + (alpha * alpha - 1) * cosThetaH * cosThetaH, gamma);
    }

    Real Smith(const Vec3 &w, const Vec3 &wh, Real alpha)
    {
        Real tanTheta = Abs(TanTheta(w));
        if(!tanTheta)
            return 1;
        if((Dot(w, wh) >= 0) != (w.z >= 0))
            return 0;
        Real root = alpha * tanTheta;
        return 2 / (1 + Sqrt(1 + root * root));
    }

    Real GTR_G(const Vec3 &wi, const Vec3 &wo, const Vec3 &wh, Real alpha)
    {
        return Smith(wi, wh, alpha) * Smith(wo, wh, alpha);;
    }

    Real GTR_NormalizedD(Real cosThetaH, Real gamma, Real alpha)
    {
        if(cosThetaH <= 0)
            return 0;

        if(gamma == 1)
        {
            Real LU = alpha * alpha - 1;
            Real LD = PI * Log_e(alpha * alpha);
            Real RD = 1 + (alpha * alpha - 1) * cosThetaH * cosThetaH;
            return LU / LD / RD;
        }

        Real LU = (gamma - 1) * (alpha * alpha - 1);
        Real LD = PI * (1 - Pow(alpha * alpha, 1 - gamma));
        Real RD = Pow(1 + (alpha * alpha - 1) * cosThetaH * cosThetaH, gamma);
        return LU / LD / RD;
    }

    std::optional<Vec3> GTR_SampleH(Real alpha, Real gamma, const Vec2 &sample)
    {
        Real phi = 2 * PI * sample.x;

        if(gamma == 1)
        {
            Real cosTheta = Sqrt((1 - Pow(alpha * alpha, 1 - sample.y)) / (1 - alpha * alpha));
            if(cosTheta < 0 || cosTheta > 1)
                return std::nullopt;
            Real sinTheta = Sqrt(1 - cosTheta * cosTheta);

            return Vec3(sinTheta * Cos(phi), sinTheta * Sin(phi), cosTheta).Normalize();
        }

        Real RUB = Pow(alpha * alpha, 1 - gamma) * (1 - sample.y) + sample.y;
        Real cosTheta = Sqrt((1 - Pow(RUB, 1 / (1 - gamma))) / (1 - alpha * alpha));
        if(cosTheta < 0 || cosTheta > 1)
            return std::nullopt;
        Real sinTheta = Sqrt(1 - cosTheta * cosTheta);

        return Vec3(sinTheta * Cos(phi), sinTheta * Sin(phi), cosTheta).Normalize();
    }

    // precondition: nWi.z > 0 && nWo.z > 0
    Spectrum GTR_BRDF(const Vec3 &nWi, const Vec3 &nWo, const Vec3 &nH, Real roughness, Real gamma, const Fresnel *fresnel)
    {
        AGZ_ASSERT(nWI.z > 0 && nWo.z > 0);

        Real D = GTR_D(nH.z, gamma, roughness * roughness);
        if(D <= 0)
            return Spectrum();

        Real cosThetaD = Dot(nWi, nH);
        Spectrum Fr = fresnel->Eval(cosThetaD);

        Real roughnessG = Real(0.5) + Real(0.5) * roughness;
        Real alphaG = roughnessG * roughnessG;
        Real G = GTR_G(nWi, nWo, nH, alphaG);

        return Fr * D * G / (4 * nWi.z * nWo.z);
    }

    struct SampleWiResult
    {
        Vec3 nWi;
        Real pdf = 0;
    };

    std::optional<SampleWiResult> GTR_SampleWi(const Vec3 &nWo, Real roughness, Real gamma, const Vec2 &sample)
    {
        if(nWo.z <= 0)
            return std::nullopt;

        Real alpha = roughness * roughness;
        auto onH = GTR_SampleH(alpha, gamma, sample);
        if(!onH)
            return std::nullopt;
        Vec3 nH = *onH;

        Vec3 nWi = (2 * Dot(nWo, nH) * nH - nWo).Normalize();
        if(nWi.z <= 0)
            return std::nullopt;

        SampleWiResult ret;
        ret.nWi = nWi;
        ret.pdf = GTR_NormalizedD(nH.z, gamma, alpha) / 4;

        return ret;
    }

    Real GTR_SampleWiPDF(const Vec3 &nWi, const Vec3 &nWo, Real alpha, Real gamma)
    {
        if(nWi.z <= 0 || nWo.z <= 0)
            return 0;
        Vec3 nH = (nWi + nWo).Normalize();
        return GTR_NormalizedD(nH.z, gamma, alpha);
    }

    Real Luminance(const Spectrum &s)
    {
        return Real(0.212671) * s.r + Real(0.715160) * s.g + Real(0.072169) * s.b
    }
}

Real DisneyPrincipledBxDF::Diffuse(const Vec3 &nWi, const Vec3 &nWo, Real cosThetaD) const
{
    Real FD90 = Real(0.5) + 2 * cosThetaD * cosThetaD * roughness_;

    Real cosThetaI = nWi.z;
    Real x = 1 - cosThetaI, x2 = x * x;
    Real a = 1 + (FD90 - 1) * (x2 * x2 * x);

    Real cosThetaO = nWo.z;
    Real y = 1 - cosThetaO, y2 = y * y;
    Real b = 1 + (FD90 - 1) * (y2 * y2 * y);

    return 1 / PI * a * b;
}

Real DisneyPrincipledBxDF::Subsurface(const Vec3 &nWi, const Vec3 &nWo, Real cosThetaD) const
{
    Real Fss90 = cosThetaD * cosThetaD * roughness_;

    Real x = 1 - nWi.z, x2 = x * x;
    Real a = 1 + (Fss90 - 1) * (x2 * x2 * x);

    Real y = 1 - nWo.z, y2 = y * y;
    Real b = 1 + (Fss90 - 1) * (y2 * y2 * y);

    return Real(1.25) / PI * (a * b * (1 / (nWo.z + nWi.z)) - Real(0.5)) + Real(0.5);
}

Spectrum DisneyPrincipledBxDF::Specular(const Vec3 &nWi, const Vec3 &nWo, const Vec3 &nH) const
{
    return GTR_BRDF(nWi, nWo, nH, roughness_, gamma_, fresnel_);
}

Real DisneyPrincipledBxDF::ClearCoat(const Vec3 &nWi, const Vec3 &nWo, const Vec3 &nH, Real cosThetaD) const
{
    if(!clearCoat_)
        return 0;

    Real roughness = Real(0.005) * (1 - clearCoatGloss_) * Real(0.1) * clearCoatGloss_;
    Real cosThetaH = nH.z;
    Real D = GTR_D(cosThetaH, gamma_, roughness * roughness);
    if(D <= 0)
        return 0;

    Real x = 1 - cosThetaD, x2 = x * x;
    Real Fr = Real(0.04) + Real(0.96) * x2 * x2 * x;
    Real G = GTR_G(nWi, nWo, nH, Real(0.25));

    return clearCoat_ * Fr * D * G / (4 * nWi.z * nWo.z);
}

DisneyPrincipledBxDF::DisneyPrincipledBxDF(
    const Spectrum &rc,
    Real roughness,
    Real specular,
    Real specularTint,
    Real metallic,
    Real sheen,
    Real sheenTint,
    Real subsurface,
    Real clearCoat,
    Real clearCoatGloss,
    const Fresnel *fresnel) noexcept
    : BxDF(BSDFType(BSDF_REFLECTION | BSDF_GLOSSY)),
      rc_            (rc),
      roughness_     (roughness),
      specular_      (specular),
      specularTint_  (specularTint),
      metallic_      (metallic),
      sheen_         (sheen),
      sheenTint_     (sheenTint),
      subsurface_    (subsurface),
      clearCoat_     (clearCoat),
      clearCoatGloss_(clearCoatGloss),
      gamma_(1.7), // IMPROVE: 暂时写成定值，理论上可取的范围是[1, 2]
      fresnel_(fresnel)
{

}

AGZ::Math::Color3<Real> DisneyPrincipledBxDF::GetAlbedo() const noexcept
{
    return rc_;
}

Spectrum DisneyPrincipledBxDF::Eval(const Vec3 &wi, const Vec3 &wo, bool star) const noexcept
{
    if(wi.z <= 0 || wo.z <= 0 || !geoInShd.InPositiveHemisphere(wi))
        return Spectrum();

    Vec3 nWi = wi.Normalize(), nWo = wo.Normalize();
    Vec3 nH = (nWi + nWo).Normalize();
    Real cosThetaD = Dot(nWo, nH);

    Real rcUTint = Luminance(rc_);
    Spectrum specularRc = specularTint_ * rc_ + (1 - specularTint_) * Spectrum(rcUTint);
    Spectrum sheenRc = sheenTint_ * rc_ + (1 - sheenTint_) * Spectrum(rcUTint);

    Real metallicT = cosThetaD * cosThetaD * cosThetaD;
    specularRc = (1 - metallicT) * specularRc + metallicT * rcUTint;

    Real x = 1 - cosThetaD, x2 = x * x;
    Real f0 = x2 * x2 * x;
    Real f1 = FresnelConductor(Spectrum(Real(1)), Spectrum(Real(0.4)), Spectrum(Real(1.6))).Eval(cosThetaD).r;
    Real sheenFactor = metallic_ * f1 + (1 - metallic_) * f0;
    Spectrum sheen = sheenFactor * sheen_ * sheenRc;

    Spectrum diffuse = metallic_ == 1 ? Spectrum() : rc_ * Diffuse(nWi, nWo, cosThetaD);
    Spectrum subsurface = metallic_ == 1 ? Spectrum() : rc_ * Subsurface(nWi, nWo, cosThetaD);

    Spectrum nonMetallic = (1 - metallic_) * (subsurface_ * subsurface + (1 - subsurface_) * diffuse + sheen);
    Spectrum specular = specularRc * Specular(nWi, nWo, nH);
    Spectrum clearCoat = Spectrum(ClearCoat(nWi, nWo, nH, cosThetaD));

    return nonMetallic + specular + clearCoat;
}

std::optional<BxDF::SampleWiResult> DisneyPrincipledBxDF::SampleWi(const Vec3 &wo, bool star, const Vec3 &sample) const noexcept
{
    if(wo.z <= 0)
        return std::nullopt;
    Vec3 nWo = wo.Normalize();

    if(clearCoat_)
    {
        Real clearCoatWeight = clearCoat_ / (clearCoat_ + Luminance(rc_));
        float x = 1.0f - nWo.z, x2 = x * x;
        Real clearCoatFresnel = Real(0.04) + Real(0.96) * x2 * x2 * x;
        Real clearCoatProb = (clearCoatFresnel * clearCoatWeight)
            / (clearCoatFresnel * clearCoatWeight + (1 - clearCoatFresnel) * (1 - clearCoatWeight));
        
        if(sample.x < clearCoatProb)
        {
            Real clearCoatRoughness = Real(0.005) * (1 - clearCoatGloss_) * Real(0.1) * clearCoatGloss_;
            auto sRet = GTR_SampleWi(nWo, clearCoatRoughness, gamma_, sample.yz());
            if(!sRet)
                return std::nullopt;
            auto [nWi, pdf] = *sRet;

            SampleWiResult ret;
            ret.wi      = nWi;
            ret.coef    = Eval(geoInShd, nWi, nWo, star);
            ret.isDelta = false;
            ret.pdf     = SampleWiPDF(geoInShd, nWi, nWo, star);
            ret.type    = BSDFType(BSDF_REFLECTION | BSDF_GLOSSY);

            return ret;
        }


    }
}

Real DisneyPrincipledBxDF::SampleWiPDF(const Vec3 &wi, const Vec3 &wo, bool star) const noexcept
{
    // TODO
    return 0;
}

} // namespace Atrc
*/