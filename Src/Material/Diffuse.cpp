#include "Diffuse.h"

AGZ_NS_BEG(Atrc)

DiffuseBRDF::DiffuseBRDF(const Spectrum &color)
    : color_(color)
{

}

BxDFType DiffuseBRDF::GetType() const
{
    return CombineBxDFTypes(BxDFType::Reflection, BxDFType::Diffuse);
}

Spectrum DiffuseBRDF::Eval(
    const SurfaceLocal &sl, const Vec3r &wi, const Vec3r &wo) const
{
    return color_;
}

Option<BxDFSample> DiffuseBRDF::Sample(
    const SurfaceLocal &sl, const Vec3r &wo, SampleSeq2D &samSeq,
    BxDFType type) const
{
    if(Dot(wo, sl.normal) <= Real(0))
        return None;

    if(HasBxDFType(type, BxDFType::Reflection))
    {
        BxDFSample ret;
        ret.coef = color_;

        if(HasBxDFType(type, BxDFType::Diffuse))
        {
            ret.dir = TransformBase(
                UniformlySampleOnHemisphere::Sample(samSeq.Next()),
                sl.ex, sl.ey, sl.ez);
            ret.pdf = UniformlySampleOnHemisphere::PDF(ret.dir);
            return ret;
        }

        ret.dir = ReflectedDirection(sl.normal, wo);
        ret.pdf = Real(1);
        return ret;
    }

    return None;
}

Real DiffuseBRDF::PDF(
    const SurfaceLocal &sl, const Vec3r &samDir, const Vec3r &wo) const
{
    return UniformlySampleOnHemisphere::PDF(samDir);
}

DiffuseMaterial::DiffuseMaterial(const Spectrum &albedo)
    : color_(SpectrumScalar(1) / PI<SpectrumScalar> * albedo)
{

}

BxDF *DiffuseMaterial::GetBxDF(const SurfaceLocal &sl) const
{
    return new DiffuseBRDF(color_);
}

AGZ_NS_END(Atrc)
