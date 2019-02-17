#include <Atrc/Lib/Material/BxDF/BxDF_Microfacet.h>

namespace Atrc
{

namespace
{
    Real GGX_D(const Vec3 &H, Real alpha)
    {
        AGZ_ASSERT(ApproxEq(H.Length(), Real(1), EPS));
        if(H.z <= 0)
            return 0;
        Real tanTheta2 = TanTheta2(H), cosTheta2 = H.z * H.z;
        Real root = alpha / (cosTheta2 * (alpha * alpha + tanTheta2));
        return root * root / PI;
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

    Vec2 Slope(Real thetaI, Vec2 sample)
    {
        Vec2 ret;

        if(thetaI < EPS)
        {
            Real R = Sqrt(Max(sample.x / ((1 - sample.x) + EPS), Real(0)));
            Real phi = 2 * PI * sample.y;
            return Vec2(R * Cos(phi), R * Sin(phi));
        }

        Real tanThetaI = Tan(thetaI);
        Real a = 1 / tanThetaI;
        Real G1 = 2 / (1 + Sqrt(Max(Real(1) + 1 / (a * a), Real(0))));

        Real A = 2 * sample.x / G1 - 1;
        if(Abs(A) == 1)
            A -= (A >= 0 ? 1 : -1) * EPS;

        Real temp = 1 / (A * A - 1);
        Real B = tanThetaI;
        Real D = Sqrt(Max(B * B * temp * temp - (A * A - B * B) * temp, Real(0)));
        Real X1 = B * temp - D, X2 = B * temp + D;
        ret.x = (A < 0 || X2 > 1 / tanThetaI) ? X1 : X2;

        Real S;
        if(sample.y > Real(0.5))
        {
            S = 1;
            sample.y = 2 * (sample.y - Real(0.5));
        }
        else
        {
            S = -1.;
            sample.y = 2 * (Real(0.5) - sample.y);
        }

        float z = (sample.y * (sample.y * (sample.y * Real(-0.365728915865723) + Real(0.790235037209296)) - Real(0.424965825137544)) + Real(0.000152998850436920)) /
                  (sample.y * (sample.y * (sample.y * (sample.y * Real(0.169507819808272) - Real(0.397203533833404)) - Real(0.232500544458471)) + 1) - Real(0.539825872510702));

        ret.y = S * z * Sqrt(1 + ret.x * ret.x);
        return ret;
    }

    Real SampleHPDF(const Vec3 &nWi, const Vec3 &H, Real alpha)
    {
        Real D = GGX_D(H, alpha);
        return Smith(nWi, H, alpha) * Abs(Dot(nWi, H)) * D / (Abs(CosTheta(nWi)) + EPS);
    }

    std::pair<Vec3, Real> SampleH(const Vec3 &nWi, Real alpha, const Vec2 &sample)
    {
        Vec3 wi = Vec3(alpha * nWi.x, alpha * nWi.y, nWi.z).Normalize();
        Real theta = Arccos(Clamp<Real>(wi.z, -1, 1));
        Real phi = Phi(wi);
        Real sinPhi = Sin(phi), cosPhi = Cos(phi);

        Vec2 slope = Slope(theta, sample);
        slope = Vec2(
            cosPhi * slope.x - sinPhi * slope.y,
            sinPhi * slope.x + cosPhi * slope.y);
        slope *= alpha;

        Real nor = 1 / Sqrt(slope.LengthSquare() + 1);
        Vec3 H = Vec3(-slope.x * nor, -slope.y * nor, nor);

        return { H, SampleHPDF(nWi, H, alpha) };
    }

    std::optional<Vec3> GetRefractDirection(const Vec3 &wi, const Vec3 &nor, Real eta)
    {
        Real cosThetaI = Dot(wi, nor);
        if(cosThetaI < 0)
            eta = 1 / eta;
        float cosThetaTSqr = 1 - (1 - cosThetaI * cosThetaI) * (eta*eta);
        if(cosThetaTSqr <= 0.0f)
            return std::nullopt;
        float sign = cosThetaI >= 0.0f ? 1.0f : -1.0f;
        return nor * (-cosThetaI * eta + sign * Sqrt(cosThetaTSqr)) + wi * eta;
    }
} // namespace anonymous

BxDF_MicrofacetReflection::BxDF_MicrofacetReflection(const Spectrum &rc, Real roughness, const Fresnel *fresnel) noexcept
    : BxDF(BSDFType(BSDF_GLOSSY | BSDF_REFLECTION)), rc_(rc), alpha_(roughness * roughness), fresnel_(fresnel)
{
    AGZ_ASSERT(roughness > 0 && fresnel);
}

Spectrum BxDF_MicrofacetReflection::GetAlbedo() const noexcept
{
    return rc_;
}

Spectrum BxDF_MicrofacetReflection::Eval(const CoordSystem &geoInShd, const Vec3 &wi, const Vec3 &wo, bool star) const noexcept
{
    if(wi.z <= 0 || wo.z <= 0 || !geoInShd.InPositiveHemisphere(wi) || !geoInShd.InPositiveHemisphere(wo))
        return Spectrum();

    Vec3 nWi = wi.Normalize(), nWo = wo.Normalize();
    Vec3 H = (nWi + nWo).Normalize();

    Real G = Smith(nWi, H, alpha_) * Smith(nWo, H, alpha_);
    Real D = GGX_D(H, alpha_);
    if(!G || !D)
        return Spectrum();
    Spectrum Fr = fresnel_->Eval(Dot(nWi, H));

    return rc_ * Fr * D * G / (4 * nWi.z * nWo.z);
}

std::optional<BxDF::SampleWiResult> BxDF_MicrofacetReflection::SampleWi(const CoordSystem &geoInShd, const Vec3 &wo, bool star, const Vec3 &sample) const noexcept
{
    if(wo.z <= 0 || !geoInShd.InPositiveHemisphere(wo))
        return std::nullopt;

    Vec3 nWo = wo.Normalize();
    auto [H, Hpdf] = SampleH(nWo, alpha_, sample.xy());

    if(Hpdf < EPS || Abs(Dot(nWo, H)) < EPS)
        return std::nullopt;

    Vec3 nWi = (2 * Dot(nWo, H) * H - nWo).Normalize();
    if(nWi.z <= 0)
        return std::nullopt;

    SampleWiResult ret;
    ret.wi      = nWi;
    ret.pdf     = Hpdf / (4 * Abs(Dot(nWi, H)));
    ret.coef    = Eval(geoInShd, nWi, nWo, star);
    ret.isDelta = false;
    ret.type    = BSDFType(BSDF_REFLECTION | BSDF_GLOSSY);

    return ret;
}

Real BxDF_MicrofacetReflection::SampleWiPDF(const CoordSystem &geoInShd, const Vec3 &wi, const Vec3 &wo, [[maybe_unused]] bool star) const noexcept
{
    if(wi.z <= 0 || wo.z <= 0 || !geoInShd.InPositiveHemisphere(wi) || !geoInShd.InPositiveHemisphere(wo))
        return 0;

    Vec3 nWi = wi.Normalize(), nWo = wo.Normalize();
    Vec3 nWh = (nWo + nWi).Normalize();

    return SampleHPDF(nWi, nWh, alpha_) / (4 * Abs(Dot(nWi, nWh)));
}

BxDF_Microfacet::BxDF_Microfacet(const Spectrum &rc, Real roughness, const Dielectric *dielectric) noexcept
    : BxDF(BSDFType(BSDF_GLOSSY | BSDF_REFLECTION | BSDF_TRANSMISSION)),
      rc_(rc), alpha_(roughness * roughness), dielectric_(dielectric)
{
    AGZ_ASSERT(roughness > 0 && dielectric);
}

Spectrum BxDF_Microfacet::GetAlbedo() const noexcept
{
    return rc_;
}

Spectrum BxDF_Microfacet::Eval(const CoordSystem &geoInShd, const Vec3 &wi, const Vec3 &wo, bool star) const noexcept
{
    Vec3 nWi = wi.Normalize(), nWo = wo.Normalize();
    Real factor = nWi.z * nWo.z;
    if(!factor)
        return Spectrum();

    bool isReflection = factor > 0;
    bool isEntering = nWo.z > 0;

    Real etaI = dielectric_->GetEtaI(), etaT = dielectric_->GetEtaT();
    if(!isEntering)
        std::swap(etaI, etaT);

    Vec3 nH;

    if(isReflection)
    {
        nH = nWi + nWo;
        if(nH.Length() < EPS)
            return Spectrum();
        nH = nH.Normalize();
    }
    else
        nH = -(etaI * nWo + etaT * nWi).Normalize();

    if(nH.z < 0)
        nH = -nH;

    Real D = GGX_D(nH, alpha_);
    if(!D)
        return Spectrum();

    Real Fr = dielectric_->Eval(Dot(nWo, nH)).r;
    Real G = Smith(nWi, nH, alpha_) * Smith(nWo, nH, alpha_);

    if(isReflection)
        return rc_ * Abs(Fr * G / (4 * nWo.z * nWo.z));

    Real HO = Dot(nH, nWo), HI = Dot(nH, nWi);
    Real sdem = etaI * HO + etaT * HI;
    Real val =  (1 - Fr) * D * G * etaT * etaT * HO * HI / (sdem * sdem * nWo.z * nWi.z);

    // TODO star
    return rc_ * Abs(val);
}

std::optional<BxDF::SampleWiResult> BxDF_Microfacet::SampleWi(const CoordSystem &geoInShd, const Vec3 &wo, bool star, const Vec3 &sample) const noexcept
{
    Vec3 nWo = wo.Normalize();
    
    auto [H, Hpdf] = SampleH(nWo.z > 0 ? nWo : -nWo, alpha_, sample.xy());
    if(!Hpdf)
        return std::nullopt;

    Real D = GGX_D(H, alpha_);
    if(!D)
        return std::nullopt;

    Real Fr = dielectric_->Eval(Dot(nWo, H)).r;
    
    if(sample.z <= Fr) // reflection
    {
        Vec3 nWi = (2 * Dot(nWo, H) * H - nWo).Normalize();
        if((nWi.z >= 0) != (nWo.z >= 0))
            return std::nullopt;

        Real pdf = Hpdf * Fr / Abs(4 * Dot(nWi, H));
        Real G = Smith(nWi, H, alpha_) * Smith(nWo, H, alpha_);

        SampleWiResult ret;
        ret.coef    = rc_ * Fr * D * G / Abs(4 * nWi.z * nWo.z);
        ret.pdf     = pdf;
        ret.isDelta = false;
        ret.type    = BSDFType(BSDF_REFLECTION | BSDF_GLOSSY);
        ret.wi      = nWi;

        return ret;
    }

    auto oWi = GetRefractDirection(-nWo, H, dielectric_->GetEtaT() / dielectric_->GetEtaI());
    if(!oWi || ((oWi->z >= 0) == (nWo.z >= 0)))
        return std::nullopt;
    Vec3 nWi = oWi->Normalize();

    bool isEntering = nWo.z > 0;
    Real etaT = dielectric_->GetEtaT(), etaI = dielectric_->GetEtaI();
    if(!isEntering)
        std::swap(etaT, etaI);

    Real HO = Dot(H, nWo), HI = Dot(H, nWi);
    Real sdem = etaI * HO + etaT * HI;
    if(!sdem)
        return std::nullopt;

    Real dHdWi = etaT * etaT * HI / (sdem * sdem);
    Real G = Smith(nWi, H, alpha_) * Smith(nWo, H, alpha_);
    Real val = (1 - Fr) * D * G * etaT * etaT * HO * HI / (sdem * sdem * nWo.z * nWi.z);

    // TODO star
    SampleWiResult ret;
    ret.wi      = nWi;
    ret.coef    = rc_ * Abs(val);
    ret.isDelta = false;
    ret.pdf     = Abs(Hpdf * dHdWi * (1 - Fr));
    ret.type    = BSDFType(BSDF_TRANSMISSION | BSDF_GLOSSY);

    return ret;
}

Real BxDF_Microfacet::SampleWiPDF(const CoordSystem &geoInShd, const Vec3 &wi, const Vec3 &wo, bool star) const noexcept
{
    Vec3 nWi = wi.Normalize(), nWo = wo.Normalize();
    Real factor = nWi.z * nWo.z;
    if(!factor)
        return 0;

    bool isReflection = factor > 0;
    bool isEntering = nWo.z > 0;

    Vec3 nH;
    Real dHdWi;
    if(isReflection)
    {
        nH = (nWo + nWi).Normalize();
        dHdWi = 1 / (4 * Dot(nWi, nH));
    }
    else
    {
        Real etaI = dielectric_->GetEtaI(), etaT = dielectric_->GetEtaT();
        if(!isEntering)
            std::swap(etaI, etaT);
        nH = -(etaI * nWo + etaT * nWi).Normalize();

        Real HO = Dot(nWo, nH), HI = Dot(nWi, nH);
        Real sdem = etaI * HO + etaT * HI;
        dHdWi = (etaT * etaT * HI) / (sdem * sdem);
    }

    if(nH.z < 0)
        nH = -nH;

    Real Hpdf = SampleHPDF(nWo.z > 0 ? nWo : -nWo, nH, alpha_);
    Real Fr = dielectric_->Eval(Dot(nWo, nH)).r;
    Real Fpdf = isReflection ? Fr : 1 - Fr;

    return Abs(Hpdf * Fpdf * dHdWi);
}

} // namespace Atrc
