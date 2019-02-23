#include <Atrc/Lib/Material/DisneyPrincipledBRDF.h>
#include <Atrc/Lib/Material/Utility/MaterialHelper.h>

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

    static constexpr BSDFType TYPE = BSDFType(BSDF_REFLECTION | BSDF_NONESPECULAR);

    std::pair<Real, Real> GetAnisotropicAlpha() const
    {
        Real a = Sqrt(1 - Real(0.9) * anisotropic_);
        Real ax = roughness_ * roughness_ / a;
        Real ay = roughness_ * roughness_ * a;
        return { ax, ay };
    }

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

        Spectrum Fs = Fr_specular(Ctint, baseColor_, specular_, specularTint_, metallic_, FD);
        Real Gs = SmithGTR2(sinPhiI, cosPhiI, tanThetaI, ax, ay) * SmithGTR2(sinPhiO, cosPhiO, tanThetaO, ax, ay);
        Real Ds = D_specular(sinPhiH, cosPhiH, sinThetaH, cosThetaH, ax, ay);

        // TODO
        return Spectrum();
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
          subsurface_(subsurface),
          metallic_(metallic),
          specular_(specular),
          specularTint_(specularTint),
          roughness_(roughness),
          anisotropic_(anisotropic),
          sheen_(sheen),
          sheenTint_(sheenTint),
          clearcoat_(clearcoat),
          clearcoatGloss_(clearcoatGloss)
    {
        
    }

    Spectrum GetAlbedo(BSDFType type) const noexcept override
    {
        if(Contains(type, TYPE))
            return baseColor_;
        return Spectrum();
    }

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

    std::optional<SampleWiResult> SampleWi(
        const CoordSystem &shd, const CoordSystem &geo,
        const Vec3 &wo, BSDFType type, bool star, const Vec3 &sample) const noexcept override
    {
        // TODO
        return std::nullopt;
    }

    Real SampleWiPDF(
        const CoordSystem &shd, const CoordSystem &geo,
        const Vec3 &wi, const Vec3 &wo, BSDFType type, bool star) const noexcept override
    {
        // TODO
        return 0;
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
