#include "../Core/Entity.h"
#include "Diffuse.h"
#include "FresnelFormulae.h"

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
    const EntityIntersection &inct, const Vec3r &wi, const Vec3r &wo) const
{
    return color_;
}

Option<BxDFSample> DiffuseBRDF::Sample(
    const EntityIntersection &inct, const Vec3r &wo, SampleSeq2D &samSeq,
    BxDFType type) const
{
    const SurfaceLocal &sl = inct.geoInct.local;

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

        if(HasBxDFType(type, BxDFType::Specular))
        {
            ret.dir = FresnelFormulae::ReflectedDirection(sl.normal, wo);
            ret.pdf = Real(1);
            return ret;
        }
    }

    return None;
}

Real DiffuseBRDF::PDF(
    const EntityIntersection &inct, const Vec3r &samDir, const Vec3r &wo) const
{
    return UniformlySampleOnHemisphere::PDF(samDir);
}

DiffuseMaterial::DiffuseMaterial(const Spectrum &albedo)
    : color_(SpectrumScalar(1) / PI<SpectrumScalar> * albedo)
{

}

RC<BxDF> DiffuseMaterial::GetBxDF(const SurfaceLocal &sl) const
{
    return MakeRC<DiffuseBRDF>(color_);
}

AGZ_NS_END(Atrc)
