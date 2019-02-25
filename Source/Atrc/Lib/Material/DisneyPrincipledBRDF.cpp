#include <Atrc/Lib/Material/DisneyPrincipledBRDF.h>
#include <Atrc/Lib/Material/Utility/MaterialHelper.h>

using namespace Atrc;

float FresnelConductor(float cosi, const float& eta, const float k)
{
    float tmp = (eta*eta + k * k) * cosi * cosi;
    float Rparl2 = (tmp - (2.f * eta * cosi) + 1) /
        (tmp + (2.f * eta * cosi) + 1);
    float tmp_f = eta * eta + k * k;
    float Rperp2 =
        (tmp_f - (2.f * eta * cosi) + cosi * cosi) /
        (tmp_f + (2.f * eta * cosi) + cosi * cosi);

    return 0.5f * (Rparl2 + Rperp2);
}

Vec3 Reflect(const Vec3& vInci, const Vec3& nNorm)
{
    return vInci + Vec3(2 * Dot(-vInci, nNorm) * nNorm);
}

Real CosTheta2(const Vec3 &w) {
    return CosTheta(w) * CosTheta(w);
}

float GGX_D(const Vec3& wh, float alpha)
{
    if(wh.z <= 0.0f)
        return 0.0f;

    const float tanTheta2 = TanTheta2(wh),
        cosTheta2 = CosTheta2(wh);

    const float root = alpha / (cosTheta2 * (alpha * alpha + tanTheta2));

    return float(1 / PI) * (root * root);
}

Vec3 GGX_SampleNormal(float u1, float u2, float* pPdf, float alpha)
{
    float alphaSqr = alpha * alpha;
    float tanThetaMSqr = alphaSqr * u1 / (1.0f - u1 + 1e-10f);
    float cosThetaH = 1.0f / std::sqrt(1 + tanThetaMSqr);

    float cosThetaH2 = cosThetaH * cosThetaH,
        cosThetaH3 = cosThetaH2 * cosThetaH,
        temp = alphaSqr + tanThetaMSqr;

    *pPdf = float(1 / PI) * alphaSqr / (cosThetaH3 * temp * temp);

    float sinThetaH = Sqrt(Max(0.0f, 1.0f - cosThetaH2));
    float phiH = u2 * float(2 * PI);

    return { sinThetaH * Cos(phiH), sinThetaH * Sin(phiH), cosThetaH };
}

float SmithG(const Vec3 & v, const Vec3& wh, float alpha)
{
    const float tanTheta = Abs(TanTheta(v));

    if(tanTheta == 0.0f)
        return 1.0f;

    if(Dot(v, wh) * CosTheta(v) <= 0)
        return 0.0f;

    const float root = alpha * tanTheta;
    return 2.0f / (1.0f + std::sqrt(1.0f + root * root));
}

float GGX_G(const Vec3& wo, const Vec3& wi, const Vec3& wh, float alpha)
{
    return SmithG(wo, wh, alpha) * SmithG(wi, wh, alpha);
}

float GGX_Pdf(const Vec3& wh, float alpha)
{
    return GGX_D(wh, alpha) * CosTheta(wh);
}

void SinCos(Real rad, Real &s, Real&c)
{
    s = Sin(rad);
    c = Cos(rad);
}

Vec2 ImportanceSampleGGX_VisibleNormal_Unit(float thetaI, float u1, float u2)
{
    Vec2 Slope;

    // Special case (normal incidence)
    if(thetaI < 1e-4f)
    {
        float SinPhi, CosPhi;
        float R = Sqrt(Max(u1 / ((1 - u1) + 1e-6f), 0.0f));
        SinCos(2 * float(PI) * u2, SinPhi, CosPhi);
        return Vec2(R * CosPhi, R * SinPhi);
    }

    // Precomputations
    float TanThetaI = tan(thetaI);
    float a = 1 / TanThetaI;
    float G1 = 2.0f / (1.0f + Sqrt(Max(1.0f + 1.0f / (a*a), 0.0f)));

    // Simulate X component
    float A = 2.0f * u1 / G1 - 1.0f;
    if(Abs(A) == 1)
        A -= (A >= 0.0f ? 1.0f : -1.0f) * 1e-4f;

    float Temp = 1.0f / (A*A - 1.0f);
    float B = TanThetaI;
    float D = Sqrt(Max(B*B*Temp*Temp - (A*A - B * B) * Temp, 0.0f));
    float Slope_x_1 = B * Temp - D;
    float Slope_x_2 = B * Temp + D;
    Slope.x = (A < 0.0f || Slope_x_2 > 1.0f / TanThetaI) ? Slope_x_1 : Slope_x_2;

    // Simulate Y component
    float S;
    if(u2 > 0.5f)
    {
        S = 1.0f;
        u2 = 2.0f * (u2 - 0.5f);
    }
    else
    {
        S = -1.0f;
        u2 = 2.0f * (0.5f - u2);
    }

    // Improved fit
    float z =
        (u2 * (u2 * (u2 * (-0.365728915865723) + 0.790235037209296) -
            0.424965825137544) + 0.000152998850436920) /
            (u2 * (u2 * (u2 * (u2 * 0.169507819808272 - 0.397203533833404) -
                0.232500544458471) + 1) - 0.539825872510702);

    Slope.y = S * z * sqrt(1.0f + Slope.x * Slope.x);

    return Slope;
}

Real AbsDot(const Vec3 &L, const Vec3 &R)
{
    return Abs(Dot(L, R));
}

float GGX_Pdf_VisibleNormal(const Vec3& wi, const Vec3& H, float Alpha)
{
    float D = GGX_D(H, Alpha);

    return SmithG(wi, H, Alpha) * AbsDot(wi, H) * D / (Abs(CosTheta(wi)) + 1e-4f);
}


Vec3 GGX_SampleVisibleNormal(const Vec3& _wi, float u1, float u2, float* pPdf, float Alpha)
{
    // Stretch wi
    Vec3 wi = Normalize(Vec3(
        Alpha * _wi.x,
        Alpha * _wi.y,
        _wi.z
    ));

    // Get polar coordinates
    float Theta = 0, Phi = 0;
    if(wi.z < float(0.99999f))
    {
        Theta = Arccos(wi.z);
        Phi = Arctan2(wi.y, wi.x);
    }
    float SinPhi, CosPhi;
    SinCos(Phi, SinPhi, CosPhi);

    // Simulate P22_{wi}(Slope.x, Slope.y, 1, 1)
    Vec2 Slope = ImportanceSampleGGX_VisibleNormal_Unit(Theta, u1, u2);

    // Step 3: rotate
    Slope = Vec2(
        CosPhi * Slope.x - SinPhi * Slope.y,
        SinPhi * Slope.x + CosPhi * Slope.y);

    // Unstretch
    Slope.x *= Alpha;
    Slope.y *= Alpha;

    // Compute normal
    float Normalization = (float)1 / Sqrt(Slope.x*Slope.x
        + Slope.y*Slope.y + (float) 1.0);

    Vec3 H = Vec3(
        -Slope.x * Normalization,
        -Slope.y * Normalization,
        Normalization
    );

    *pPdf = GGX_Pdf_VisibleNormal(_wi, H, Alpha);

    return H;
}

namespace Atrc
{
    
namespace
{

template<typename T>
auto Mix(const T &a, const T &b, Real t) noexcept
{
    return (1 - t) * a + t * b;
}

Real OneMinusX5(Real x) noexcept
{
    Real t = 1 - x, t2 = t * t;
    return t2 * t2 * t;
}

Real Sqr(Real x) noexcept { return x * x; }

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

Spectrum f_sheen(const Spectrum &Ctint, Real sheenTint, Real sheen, Real FD) noexcept
{
    Spectrum Csh = Mix(Spectrum(1), Ctint, sheenTint);
    return Csh * sheen * FD;
}

Real SmithGTR2(Real sinPhi, Real cosPhi, Real tanTheta, Real alphaX, Real alphaY)
{
    Real sqr = 1 + Sqr(tanTheta) * (Sqr(cosPhi * alphaX) + Sqr(sinPhi * alphaY));
    Real lambda = Real(-0.5) + Real(0.5) * Sqrt(sqr);
    return 1 / (1 + lambda);
}

Spectrum Fr_specular(const Spectrum &Ctint, const Spectrum &C, Real specular, Real specularTint, Real metallic, Real FD)
{
    Spectrum Cs = Mix(Real(0.08) * specular * Mix(Spectrum(1), Ctint, specularTint), C, metallic);
    return Cs + (Spectrum(1) - Cs) * FD;
}

Real D_specular(Real sinPhiH, Real cosPhiH, Real sinThetaH, Real cosThetaH, Real ax, Real ay)
{
    Real A = Sqr(cosPhiH / ax) + Sqr(sinPhiH / ay);
    Real RD = Sqr(sinThetaH) * A + Sqr(cosThetaH);
    return 1 / (PI * ax * ay * Sqr(RD));
}

Real SmithGTR1(Real tanTheta, Real alpha)
{
    Real lambda = -Real(0.5) + Real(0.5) * Sqrt(1 + Sqr(alpha * tanTheta));
    return 1 / (1 + lambda);
}

Real D_clearcoat(Real cosThetaH, Real sinThetaH, Real alpha)
{
    Real t = Sqr(cosThetaH * alpha) + Sqr(sinThetaH);
    Real d = 2 * PI * Log_e(alpha) * t;
    Real u = Sqr(alpha) - 1;
    return u / d;
}

#define Lerp Mix

Real Luminance(const Spectrum &c)
{
    return Real(0.2126) * c.r + Real(0.7152) * c.g + Real(0.0722) * c.b;;
}

class DisneyBRDF : public BSDF
{
    const Spectrum baseColor_;
    const Real subsurface_;
    const Real metallic_;
    const Real specular_;
    const Real specularTint_;
    const Real roughness_;
    const Real anisotropic_;
    const Real sheen_;
    const Real sheenTint_;
    const Real clearcoat_;
    const Real clearcoatGloss_;

    static constexpr BSDFType TYPE = BSDFType(BSDF_REFLECTION | BSDF_NONSPECULAR);

    std::pair<Real, Real> GetAnisotropicAlpha() const
    {
        Real a = Sqrt(1 - Real(0.9) * anisotropic_);
        Real ax = roughness_ * roughness_ / a;
        Real ay = roughness_ * roughness_ * a;
        return { ax, ay };
    }

    // wi, wo and wh must be normalized
    Spectrum EvalLocal(const Vec3 &wi, const Vec3 &wo, const Vec3 &wh) const
    {
        if(wi.z <= 0 || wo.z <= 0)
            return Spectrum();

        Real cosThetaI = wi.z, cosThetaO = wo.z, cosThetaH = wh.z;
        Real cosThetaD = Dot(wi, wh), cosThetaD2 = Sqr(cosThetaD);
        Real FI = OneMinusX5(cosThetaI), FO = OneMinusX5(cosThetaO), FD = OneMinusX5(cosThetaD);

        Real phiI = Phi(wi), thetaI = Theta(wi);
        Real phiO = Phi(wo), thetaO = Theta(wo);
        Real phiH = Phi(wh), thetaH = Theta(wh);
        Real sinPhiI = Sin(phiI), cosPhiI = Cos(phiI);
        Real sinPhiO = Sin(phiO), cosPhiO = Cos(phiO);
        Real sinPhiH = Sin(phiH), cosPhiH = Cos(phiH);
        Real sinThetaI = Sin(thetaI), tanThetaI = sinThetaI / cosThetaI;
        Real sinThetaO = Sin(thetaO), tanThetaO = sinThetaO / cosThetaO;
        Real sinThetaH = Sin(thetaH);

        Real lumC = Real(0.2126) * baseColor_.r + Real(0.7152) * baseColor_.g + Real(0.0722) * baseColor_.b;
        if(lumC < EPS)
            return Spectrum();
        Spectrum Ctint = baseColor_ / lumC;

        auto [ax, ay] = GetAnisotropicAlpha();

        Real f_d = 0, f_ss = 0;
        Spectrum f_sh;

        if(metallic_ < 1)
        {
            if(subsurface_ < 1)
                f_d = f_diffuse(FI, FO, cosThetaD2, roughness_);

            if(subsurface_ > 0)
                f_ss = f_subsurface(cosThetaI, cosThetaO, FI, FO, cosThetaD2, roughness_);

            if(sheen_ > 0)
                f_sh = f_sheen(Ctint, sheenTint_, sheen_, FD);
        }

        // specular

        Spectrum Fs = Fr_specular(Ctint, baseColor_, specular_, specularTint_, metallic_, FD);
        Real Gs = SmithGTR2(sinPhiI, cosPhiI, tanThetaI, ax, ay) * SmithGTR2(sinPhiO, cosPhiO, tanThetaO, ax, ay);
        Real Ds = D_specular(sinPhiH, cosPhiH, sinThetaH, cosThetaH, ax, ay);

        // clearcoat

        Real Fc = 0, Gc = 0, Dc = 0;

        if(clearcoat_ > 0)
        {
            Fc = Real(0.04) + Real(0.96) * FD;
            Gc = SmithGTR1(tanThetaI, Real(0.25)) * SmithGTR1(tanThetaO, Real(0.25));
            Dc = D_clearcoat(cosThetaH, sinThetaH, Mix(Real(0.1), Real(0.01), clearcoatGloss_));
        }

        return (1 - metallic_) * (baseColor_ / PI * Mix(f_d, f_ss, subsurface_) + f_sh)
             + Fs * Gs * Ds / (4 * cosThetaI * cosThetaO)
             + clearcoat_ / 4 * Fc * Gc * Dc / (4 * cosThetaI * cosThetaO);
    }

    struct SampleWiWeights
    {
        Real wd;
        Real ws;
        Real wc;
    };

    SampleWiWeights GetSampleWiWeights(const Vec3 &wo, const Vec3 &wh) const
    {
        /*Real cosThetaD = Dot(wo, wh);
        Real FD = OneMinusX5(cosThetaD);

        Real wd = 1 - metallic_;
        Real ws = Fr_specular(Spectrum(1), Spectrum(1), specular_, specularTint_, metallic_, FD).r;
        Real wc = clearcoat_ / 4 * (Real(0.04) + Real(0.96) * FD);

        return SampleWiWeights{ wd, ws, wc }.Normalize();*/
        constexpr Real V = Real(1) / 3;
        return { V, V, V };
    }

    Real SampleWiPDFLocal(const Vec3 &wi, const Vec3 &wo, const Vec3 &wh) const
    {
        if(wi.z <= 0 || wo.z <= 0)
            return 0;

        auto [ax, ay] = GetAnisotropicAlpha();

        Real cosThetaH = wh.z;
        Real phiH = Phi(wh), thetaH = Theta(wh);
        Real sinPhiH = Sin(phiH), cosPhiH = Cos(phiH);
        Real sinThetaH = Sin(thetaH);

        Real pd = AGZ::Math::DistributionTransform::ZWeightedOnUnitHemisphere<Real>::PDF(wi);
        Real ps = D_specular(sinPhiH, cosPhiH, sinThetaH, cosThetaH, ax, ay) / 4;
        Real pc = D_clearcoat(cosThetaH, sinThetaH, Mix(Real(0.1), Real(0.01), clearcoatGloss_)) / 4;

        auto weights = GetSampleWiWeights(wo, Vec3::UNIT_Z());
        return weights.wd * pd + weights.ws * ps + weights.wc * pc;
    }

public:

    DisneyBRDF(
        const Spectrum &baseColor,
        Real subsurface,
        Real metallic,
        Real specular,
        Real specularTint,
        Real roughness,
        Real anisotropic,
        Real sheen,
        Real sheenTint,
        Real clearcoat,
        Real clearcoatGloss) noexcept
        : baseColor_(baseColor),
          subsurface_(Saturate(subsurface)),
          metallic_(Saturate(metallic)),
          specular_(Saturate(specular)),
          specularTint_(Saturate(specularTint)),
          roughness_(Saturate(roughness)),
          anisotropic_(Saturate(anisotropic)),
          sheen_(Saturate(sheen)),
          sheenTint_(Saturate(sheenTint)),
          clearcoat_(Saturate(clearcoat)),
          clearcoatGloss_(Saturate(clearcoatGloss))
    {
        
    }

    Spectrum GetAlbedo(BSDFType type) const noexcept override
    {
        if(Contains(type, TYPE))
            return baseColor_;
        return Spectrum();
    }

    /*Spectrum Eval(const CoordSystem &shd, const CoordSystem &geo, const Vec3 &wi, const Vec3 &wo, BSDFType type, bool star) const noexcept override
    {
        if(!Contains(type, TYPE))
            return Spectrum();
        Vec3 lWi = shd.World2Local(wi).Normalize(), lWo = shd.World2Local(wo).Normalize();
        if(lWi.z <= 0 || lWo.z <= 0)
            return Spectrum();
        return EvalTransformed(lWo, lWi);
    }*/

    Spectrum Eval(
        const CoordSystem &shd, const CoordSystem &geo,
        const Vec3 &wi, const Vec3 &wo, BSDFType type, bool star) const noexcept override
    {
        if(!Contains(type, TYPE))
            return Spectrum();
        Vec3 lwi = shd.World2Local(wi).Normalize(), lwo = shd.World2Local(wo).Normalize();
        Vec3 wh = (lwi + lwo).Normalize();
        return EvalLocal(lwi, lwo, wh);
    }

    std::optional<SampleWiResult> SampleWi(const CoordSystem& shd, const CoordSystem& geo, const Vec3& _wo, BSDFType type, bool star, const Vec3& sample) const noexcept override
    {
        if(!Contains(type, TYPE))
        {
            return std::nullopt;
        }

        Vec3 wo = shd.World2Local(_wo).Normalize();
        Vec3 wi, wh;
        bool sampleCoat = false;
        Vec3 remappedSample = sample;

        if(clearcoat_ > 0.0f)
        {
            auto albedo = baseColor_;
            float coatWeight = clearcoat_ / (clearcoat_ + Luminance(albedo));
            float FresnelCoat = Fresnel_Schlick_Coat(Abs(CosTheta(wo)));
            float probCoat = (FresnelCoat * coatWeight) /
                (FresnelCoat * coatWeight +
                (1 - FresnelCoat) * (1 - coatWeight));

            if(sample.v < probCoat)
            {
                sampleCoat = true;
                remappedSample.v /= probCoat;

                float coatRough = Lerp(0.005f, 0.1f, clearcoat_);
                float coatPdf;
                wh = GGX_SampleVisibleNormal(wo, remappedSample.u, remappedSample.v, &coatPdf, coatRough);
                wi = Reflect(-wo, wh);
            }
            else
            {
                sampleCoat = false;
                remappedSample.v = (sample.v - probCoat) / (1.0f - probCoat);
            }
        }


        if(!sampleCoat)
        {
            float microfacetPdf;
            float roughness = roughness_;
            roughness = Clamp(roughness, 0.02f, 1.0f);

            wh = GGX_SampleVisibleNormal(wo, remappedSample.u, remappedSample.v, &microfacetPdf, roughness * roughness);

            float normalRef = Lerp(0.0f, 0.08f, specular_);
            float ODotH = Dot(wo, wh);
            float probSpec = Fresnel_Schlick(ODotH, normalRef);

            if(remappedSample.z <= probSpec)
            {
                wi = Reflect(-wo, wh);
            }
            else
            {
                wi = AGZ::Math::DistributionTransform::ZWeightedOnUnitHemisphere<Real>::Transform(remappedSample.xy()).sample;
                if(wo.z < 0.0f)
                    wi.z *= -1.0f;
            }
        }

        if(wi.z <= 0.0f)
        {
            return std::nullopt;
        }

        wi = wi.Normalize();

        SampleWiResult ret;
        ret.wi = shd.Local2World(wi).Normalize();
        ret.coef = EvalLocal(wi, wo, (wi + wo).Normalize());//EvalTransformed(wo, wi);
        ret.isDelta = false;
        ret.pdf = PdfInner(wo, wi);
        ret.type = TYPE;
        return ret;

        /**pvIn = diffGeom.LocalToWorld(wi);
        *pPdf = PdfInner(wo, wi, diffGeom, types);

        if(Math::Dot(_wo, diffGeom.mGeomNormal) * Math::Dot(*pvIn, diffGeom.mGeomNormal) > 0.0f)
            types = ScatterType(types & ~BSDF_TRANSMISSION);
        else
            types = ScatterType(types & ~BSDF_REFLECTION);

        if(!MatchesTypes(types))
        {
            *pPdf = 0.0f;
            return Color::BLACK;
        }

        return EvalTransformed(wo, wi, diffGeom, types);*/
    }

    Real SampleWiPDF(const CoordSystem& shd, const CoordSystem& geo, const Vec3& wi, const Vec3& wo, BSDFType type, bool star) const noexcept override
    {
        Vec3 lWi = shd.World2Local(wi).Normalize(), lWo = shd.World2Local(wo).Normalize();
        return PdfInner(lWo, lWi);
    }

    float PdfInner(const Vec3& wo, const Vec3& wi) const
    {
        Vec3 wh = Normalize(wo + wi);
        if(wh == Vec3())
            return 0.0f;

        Real roughness = roughness_;
        roughness = Clamp(roughness, 0.02f, 1.0f);

        float microfacetPdf = GGX_Pdf_VisibleNormal(wo, wh, roughness * roughness);
        float pdf = 0.0f;
        float dwh_dwi = 1.0f / (4.0f * AbsDot(wi, wh));
        float specPdf = microfacetPdf * dwh_dwi;

        float normalRef = Lerp(0.0f, 0.08f, specular_);
        float ODotH = Dot(wo, wh);
        float probSpec = Fresnel_Schlick(ODotH, normalRef);

        pdf += specPdf * probSpec;
        pdf += Abs(CosTheta(wi)) * float(1 / PI) * (1 - probSpec);

        if(clearcoat_ > 0.0f)
        {
            Spectrum albedo = baseColor_;
            float coatWeight = clearcoat_/ (clearcoat_+ Luminance(albedo));
            float FresnelCoat = Fresnel_Schlick_Coat(Abs(CosTheta(wo)));
            float probCoat = (FresnelCoat * coatWeight) /
                (FresnelCoat * coatWeight +
                (1 - FresnelCoat) * (1 - coatWeight));
            float coatRough = Lerp(0.005f, 0.10f, clearcoatGloss_);
            float coatHalfPdf = GGX_Pdf_VisibleNormal(wo, wh, coatRough);
            float coatPdf = coatHalfPdf * dwh_dwi;

            pdf *= 1.0f - probCoat;
            pdf += coatPdf * probCoat;
        }

        return pdf;
    }

    Spectrum EvalTransformed(const Vec3& wo, const Vec3& wi) const
    {
        Spectrum albedo = baseColor_;
        float roughness = roughness_;
        roughness = Clamp(roughness, 0.02f, 1.0f);
        Spectrum UntintedSpecAlbedo = Spectrum(Luminance(albedo));
        Spectrum specAlbedo = Lerp(Lerp(albedo, UntintedSpecAlbedo, 1.0f - specularTint_), albedo, metallic_);
        Spectrum sheenAlbedo = Lerp(UntintedSpecAlbedo, albedo, sheenTint_);

        Vec3 wh = Normalize(wo + wi);
        float ODotH = Dot(wo, wh);
        float IDotH = Dot(wi, wh);
        float OneMinusODotH = 1.0f - ODotH;
        Spectrum grazingSpecAlbedo = Lerp(specAlbedo, UntintedSpecAlbedo, metallic_);
        specAlbedo = Lerp(specAlbedo, UntintedSpecAlbedo, OneMinusODotH * OneMinusODotH * OneMinusODotH);

        // Sheen term
        float F = Fresnel_Schlick(ODotH, 0.0f);
        Spectrum sheenTerm = F * sheen_ * sheenAlbedo;

        return (1.0f - metallic_)
            * (albedo * Lerp(DiffuseTerm(wo, wi, IDotH, roughness), SubsurfaceTerm(wo, wi, IDotH, roughness), subsurface_)
                + sheenTerm)
            + specAlbedo * SpecularTerm(wo, wi, wh, ODotH, roughness, nullptr)
            + Spectrum(ClearCoatTerm(wo, wi, wh, IDotH, clearcoatGloss_));
    }

    float DiffuseTerm(const Vec3& wo, const Vec3& wi, const float IDotH, const float roughness) const
    {
        if(metallic_ == 1.0f)
            return 0.0f;

        float oneMinusCosL = 1.0f - Abs(CosTheta(wi));
        float oneMinusCosLSqr = oneMinusCosL * oneMinusCosL;
        float oneMinusCosV = 1.0f - Abs(CosTheta(wo));
        float oneMinusCosVSqr = oneMinusCosV * oneMinusCosV;
        float F_D90 = 0.5f + 2.0f * IDotH * IDotH * roughness;

        return float(1 / PI) * (1.0f + (F_D90 - 1.0f) * oneMinusCosLSqr * oneMinusCosLSqr * oneMinusCosL) *
            (1.0f + (F_D90 - 1.0f) * oneMinusCosVSqr * oneMinusCosVSqr * oneMinusCosV);
    }

    float SpecularTerm(const Vec3& wo,
        const Vec3& wi,
        const Vec3& wh,
        const float ODotH,
        const float roughness,
        const float* pFresnel = nullptr) const
    {
        if(CosTheta(wo) * CosTheta(wi) <= 0.0f)
            return 0.0f;

        float D = GGX_D(wh, roughness * roughness);
        if(D == 0.0f)
            return 0.0f;

        float normalRef = Lerp(0.0f, 0.08f, specular_);
        float F = pFresnel ? *pFresnel : Fresnel_Schlick(ODotH, normalRef);

        float roughForG = (0.5f + 0.5f * roughness);
        float G = GGX_G(wo, wi, wh, roughForG * roughForG);

        return F * D * G / (4.0f * Abs(CosTheta(wi)) * Abs(CosTheta(wo)));
    }

    float SubsurfaceTerm(const Vec3& wo, const Vec3& wi, const float IDotH, const float roughness) const
    {
        if(subsurface_ == 0.0f)
            return 0.0f;

        float oneMinusCosL = 1.0f - Abs(CosTheta(wi));
        float oneMinusCosLSqr = oneMinusCosL * oneMinusCosL;
        float oneMinusCosV = 1.0f - Abs(CosTheta(wo));
        float oneMinusCosVSqr = oneMinusCosV * oneMinusCosV;
        float F_ss90 = IDotH * IDotH * roughness;

        float S = (1.0f + (F_ss90 - 1.0f) * oneMinusCosLSqr * oneMinusCosLSqr * oneMinusCosL) *
            (1.0f + (F_ss90 - 1.0f) * oneMinusCosVSqr * oneMinusCosVSqr * oneMinusCosV);

        return float(1 / PI) * 1.25f * (S * (1.0f / (Abs(CosTheta(wo)) + Abs(CosTheta(wi))) - 0.5f) + 0.5f);
    }

    float ClearCoatTerm(const Vec3& wo, const Vec3& wi, const Vec3& wh, const float IDotH, const float roughness) const
    {
        if(clearcoat_ == 0.0f)
            return 0.0f;

        float rough = Lerp(0.005f, 0.1f, roughness);

        float D = GGX_D(wh, rough);
        if(D == 0.0f)
            return 0.0f;

        float oneMinusCosD = 1.0f - IDotH;
        float oneMinusCosDSqr = oneMinusCosD * oneMinusCosD;
        float F = Fresnel_Schlick_Coat(IDotH);
        float G = GGX_G(wo, wi, wh, 0.25f);

        return clearcoat_ * D * F * G / (4.0f * Abs(CosTheta(wi)) * Abs(CosTheta(wo)));
    }

    float Fresnel_Schlick(const float cosD, const float normalReflectance) const
    {
        float oneMinusCosD = 1.0f - cosD;
        float oneMinusCosDSqr = oneMinusCosD * oneMinusCosD;
        float fresnel = normalReflectance +
            (1.0f - normalReflectance) * oneMinusCosDSqr * oneMinusCosDSqr * oneMinusCosD;
        float fresnelConductor = FresnelConductor(cosD, 0.4f, 1.6f);

        return Lerp(fresnel, fresnelConductor, metallic_);
    }

    float Fresnel_Schlick_Coat(const float cosD) const
    {
        float oneMinusCosD = 1.0f - cosD;
        float oneMinusCosDSqr = oneMinusCosD * oneMinusCosD;
        float fresnel = 0.04f +
            (1.0f - 0.04f) * oneMinusCosDSqr * oneMinusCosDSqr * oneMinusCosD;

        return fresnel;
    }
};

} // namespace null

DisneyBRDFMaterial::DisneyBRDFMaterial(
    const Texture *baseColor,
    const Texture *subsurface,
    const Texture *metallic,
    const Texture *specular,
    const Texture *specularTint,
    const Texture *roughness,
    const Texture *anisotropic,
    const Texture *sheen,
    const Texture *sheenTint,
    const Texture *clearcoat,
    const Texture *clearcoatGloss,
    const NormalMapper *normalMapper) noexcept
    : baseColor_(baseColor),
      subsurface_(subsurface),
      metallic_(metallic),
      specular_(specular),
      specularTint_(specularTint),
      roughness_(roughness),
      anisotropic_(anisotropic),
      sheen_(sheen),
      sheenTint_(sheenTint),
      clearcoat_(clearcoat),
      clearcoatGloss_(clearcoatGloss),
      normalMapper_(normalMapper)
{
    
}

ShadingPoint DisneyBRDFMaterial::GetShadingPoint(const Intersection &inct, Arena &arena) const
{
    ShadingPoint ret;
    ret.uv = inct.usr.uv;
    ret.coordSys = MatHelper::ComputeShadingCoordSystem(inct, normalMapper_);

    Spectrum baseColor  = baseColor_     ->Sample(ret.uv);
    Real subsurface     = subsurface_    ->Sample1(ret.uv);
    Real metallic       = metallic_      ->Sample1(ret.uv);
    Real specular       = specular_      ->Sample1(ret.uv);
    Real specularTint   = specularTint_  ->Sample1(ret.uv);
    Real roughness      = roughness_     ->Sample1(ret.uv);
    Real anisotropic    = anisotropic_   ->Sample1(ret.uv);
    Real sheen          = sheen_         ->Sample1(ret.uv);
    Real sheenTint      = sheenTint_     ->Sample1(ret.uv);
    Real clearcoat      = clearcoat_     ->Sample1(ret.uv);
    Real clearcoatGloss = clearcoatGloss_->Sample1(ret.uv);

    auto bsdf = arena.Create<DisneyBRDF>(
        baseColor, subsurface, metallic, specular, specularTint,
        roughness, anisotropic, sheen, sheenTint, clearcoat, clearcoatGloss);
    ret.bsdf = bsdf;

    return ret;
}

} // namespace Atrc
