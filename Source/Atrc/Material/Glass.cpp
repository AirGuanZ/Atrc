#include <Atrc/Material/Glass.h>

AGZ_NS_BEG(Atrc)

namespace
{
    bool Refract(const Vec3r &wi, const Vec3r &nor, Real niDivNt, Vec3r *refracted)
    {
        Real t = -Dot(wi, nor);
        Real jdg = 1.0 - niDivNt * niDivNt * (1 - t * t);
        if(jdg > 0.0)
        {
            *refracted = niDivNt * (-wi - t * nor) - Sqrt(jdg) * nor;
            return true;
        }
        return false;
    }

    Real ChristopheSchlick(Real cos, Real refIdx)
    {
        Real t = (1 - refIdx) / (1 + refIdx);
        t = t * t;
        return t + (1 - t) * Pow(1 - cos, 5);
    }
}

GlassBxDF::GlassBxDF(
    const Intersection &inct, const Spectrum &reflColor, const Spectrum &refrColor, Real refIdx)
    : nor_(inct.nor), reflColor_(reflColor), refrColor_(refrColor), refIdx_(refIdx)
{

}

BxDFType GlassBxDF::GetType() const
{
    return BXDF_REFLECTION | BXDF_TRANSMISSION | BXDF_SPECULAR;
}

Spectrum GlassBxDF::Eval(const Vec3r &wi, const Vec3r &wo) const
{
    return SPECTRUM::BLACK;
}

Option<BxDFSample> GlassBxDF::Sample(const Vec3r &wi, BxDFType type) const
{
    Real dot = Dot(wi, nor_), absDot = Abs(dot);
    Real niDivNt, cosine = absDot;

    Vec3r nor;
    if(dot < 0)
    {
        niDivNt = refIdx_;
        nor = -nor_;
        cosine *= refIdx_;
    }
    else
    {
        niDivNt = 1 / refIdx_;
        nor = nor_;
    }

    if(type.Contains(BXDF_TRANSMISSION | BXDF_SPECULAR))
    {
        if(!type.Contains(BXDF_REFLECTION) ||
            Rand() > ChristopheSchlick(cosine, refIdx_))
        {
            Vec3r refrDir;
            if(Refract(wi, nor, niDivNt, &refrDir))
            {
                BxDFSample ret;
                ret.dir = refrDir.Normalize();
                ret.coef = refrColor_ / SS(absDot);
                ret.pdf = 1;
                return ret;
            }
        }
    }

    if(type.Contains(BXDF_REFLECTION | BXDF_SPECULAR))
    {
        BxDFSample ret;
        ret.dir = 2 * absDot * nor - wi;
        ret.coef = reflColor_ / SS(absDot);
        ret.pdf = 1;
        return ret;
    }

    return None;
}

GlassMaterial::GlassMaterial(const Spectrum &reflColor, const Spectrum &refrColor, Real refIdx)
    : reflColor_(reflColor), refrColor_(refrColor), refIdx_(refIdx)
{

}

RC<BxDF> GlassMaterial::GetBxDF(const Intersection &inct, const Vec2r &matParam) const
{
    return NewRC<GlassBxDF>(inct, reflColor_, refrColor_, refIdx_);
}


AGZ_NS_END(Atrc)
