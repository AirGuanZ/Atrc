#include "Diffuse.h"

AGZ_NS_BEG(Atrc)

DiffuseBRDF::DiffuseBRDF(const Spectrum &color)
    : color_(color)
{

}

BxDFType DiffuseBRDF::GetType() const
{
    return CombingBxDFTypes(BxDFType::Reflection, BxDFType::Diffuse);
}

Spectrum DiffuseBRDF::Eval(const Vec3r &wi, const Vec3r &wo) const
{
    return color_;
}

Option<BxDFSample> DiffuseBRDF::Sample(const Vec3r &wo, SampleSeq2D &samSeq) const
{
    BxDFSample ret;
    ret.coef = color_;
    ret.dir = UniformlySampleOnHemisphere::Sample(samSeq.Next());
    ret.pdf = UniformlySampleOnHemisphere::PDF(ret.dir);
    return ret;
}

Real DiffuseBRDF::PDF(const Vec3r &samDir, const Vec3r &wo) const
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
