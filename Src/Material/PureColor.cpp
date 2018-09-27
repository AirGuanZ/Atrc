#include "PureColor.h"

AGZ_NS_BEG(Atrc)

PureColorBRDF::PureColorBRDF(const Spectrum &color)
    : color_(color)
{

}

BxDFType PureColorBRDF::GetType() const
{
    return BxDFType::Ambient;
}

Spectrum PureColorBRDF::Eval(const Vec3r &wi, const Vec3r &wo) const
{
    return SPECTRUM::BLACK;
}

Option<BxDFSample> PureColorBRDF::Sample(const Vec3r &wo) const
{
    return None;
}

Real PureColorBRDF::PDF(const Vec3r &wi, const Vec3r &wo) const
{
    return Real(0);
}

Spectrum PureColorBRDF::AmbientRadiance() const
{
    return color_;
}

PureColorMaterial::PureColorMaterial(const Spectrum &color)
    : color_(color)
{

}

BxDF *PureColorMaterial::GetBxDF(const SurfaceLocal &sl) const
{
    return new PureColorBRDF(color_);
}

AGZ_NS_END(Atrc)
