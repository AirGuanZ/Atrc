#include <Atrc/Material/Ambient.h>

AGZ_NS_BEG(Atrc)

AmbientBRDF::AmbientBRDF(const Spectrum &color)
    : color_(color)
{

}

BxDFType AmbientBRDF::GetType() const
{
    return BXDF_NONE;
}

Spectrum AmbientBRDF::Eval(const Vec3r &wi, const Vec3r &wo) const
{
    return SPECTRUM::BLACK;
}

Option<BxDFSample> AmbientBRDF::Sample(const Vec3r &wi, BxDFType type) const
{
    return None;
}

Spectrum AmbientBRDF::AmbientRadiance(const Intersection &inct) const
{
    return color_;
}

AmbientMaterial::AmbientMaterial(const Spectrum &color)
    : color_(color)
{
    
}

RC<BxDF> AmbientMaterial::GetBxDF(const Intersection &inct) const
{
    return NewRC<AmbientBRDF>(color_);
}

AGZ_NS_END(Atrc)
