#include "FresnelFormulae.h"
#include "IdealGlass.h"

AGZ_NS_BEG(Atrc)

IdealGlassBxDF::IdealGlassBxDF(const IdealGlassMaterial *material)
    : material_(material)
{
    AGZ_ASSERT(material);
}

BxDFType IdealGlassBxDF::GetType() const
{
    return CombineBxDFTypes(
        BxDFType::Reflection, BxDFType::Transmission, BxDFType::Specular);
}

Spectrum IdealGlassBxDF::Eval(
    const SurfaceLocal &sl, const Vec3r &wi, const Vec3r &wo) const
{
    return SPECTRUM::BLACK;
}

Option<BxDFSample> IdealGlassBxDF::Sample(
    const SurfaceLocal &sl, const Vec3r &wo,
    SampleSeq2D &samSeq, BxDFType type) const
{
    // Reflection
    if(HasBxDFType(type, CombineBxDFTypes(BxDFType::Reflection, BxDFType::Specular)))
    {
        BxDFSample ret;
        ret.coef = material_->reflectionColor_;
        ret.dir  = ReflectedDirection(sl.normal, wo);
        ret.pdf  = Real(1);
        return ret;
    }

    // Transmission
    if(HasBxDFType(type, CombineBxDFTypes(BxDFType::Transmission, BxDFType::Specular)))
    {
        // TODO: Waiting for implementation of fresnel equations
    }

    return None;
}

Real IdealGlassBxDF::PDF(
    const SurfaceLocal &sl, const Vec3r &samDir, const Vec3r &wo) const
{
    return Real(1);
}

IdealGlassMaterial::IdealGlassMaterial(
    const Spectrum &refColor, const Spectrum &transColor, Real refractivity)
    : reflectionColor_(refColor), transmissionColor_(transColor),
      refractivity_(refractivity)
{

}

BxDF *IdealGlassMaterial::GetBxDF(const SurfaceLocal &sl) const
{
    return new IdealGlassBxDF(this);
}

AGZ_NS_END(Atrc)
