#include "../Core/Entity.h"
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
    const EntityIntersection &inct, const Vec3r &wi, const Vec3r &wo) const
{
    return SPECTRUM::BLACK;
}

Option<BxDFSample> IdealGlassBxDF::Sample(
    const EntityIntersection &inct, const Vec3r &wo,
    SampleSeq2D &samSeq, BxDFType type) const
{
    // Reflection
    if(HasBxDFType(type, CombineBxDFTypes(BxDFType::Reflection, BxDFType::Specular)))
    {
        BxDFSample ret;
        ret.coef = material_->reflectionColor_;
        ret.dir  = FresnelFormulae::ReflectedDirection(
            inct.geoInct.local.normal, wo);
        ret.pdf  = Real(1);
        return ret;
    }

    // Transmission
    if(HasBxDFType(type, CombineBxDFTypes(BxDFType::Transmission, BxDFType::Specular)))
    {
        Real eta_i, eta_t;
        if(inct.geoInct.fromOutside)
        {
            eta_i = Real(1);
            eta_t = material_->refractivity_;
        }
        else
        {
            eta_i = material_->refractivity_;
            eta_t = Real(1);
        }

        auto dir = FresnelFormulae::TransmittedDirection(
            inct.geoInct.local.normal, wo, eta_i, eta_t);
        // Full reflection
        if(!dir)
        {
            BxDFSample ret;
            ret.coef = material_->transmissionColor_;
            ret.dir = FresnelFormulae::ReflectedDirection(
                inct.geoInct.local.normal, wo);
            ret.pdf = Real(1);
            return ret;
        }

        BxDFSample ret;
        ret.coef = material_->transmissionColor_;
        ret.dir = dir.value();
        ret.pdf = Real(1);
        return ret;
    }

    return None;
}

Real IdealGlassBxDF::PDF(
    const EntityIntersection &inct, const Vec3r &samDir, const Vec3r &wo) const
{
    return Real(1);
}

IdealGlassMaterial::IdealGlassMaterial(
    const Spectrum &refColor, const Spectrum &transColor, Real refractivity)
    : reflectionColor_(refColor), transmissionColor_(transColor),
      refractivity_(refractivity)
{

}

RC<BxDF> IdealGlassMaterial::GetBxDF(const SurfaceLocal &sl) const
{
    return MakeRC<IdealGlassBxDF>(this);
}

AGZ_NS_END(Atrc)
