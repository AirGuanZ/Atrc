#include <Atrc/Light/GeometricDiffuseLight.h>

AGZ_NS_BEG(Atrc)

GeometricDiffuseLight::GeometricDiffuseLight(const Geometry *geometry, const Spectrum &radiance)
    : GeometricLight(geometry), radiance_(radiance)
{
    AGZ_ASSERT(geometry);
}

LightSampleToResult GeometricDiffuseLight::SampleTo(const SurfacePoint &sp) const
{
    LightSampleToResult ret;
    GeometrySampleResult sam = geometry_->Sample();

    ret.pos = sam.pos;
    ret.wi = (sam.pos - sp.pos).Normalize();
    ret.pdf = sam.pdf;

    auto cos = Dot(sam.nor, -ret.wi);
    if(cos <= 0.0)
        ret.radiance = SPECTRUM::BLACK;
    else
        ret.radiance = radiance_ * cos / (sam.pos - sp.pos).LengthSquare();

    return ret;
}

Real GeometricDiffuseLight::SampleToPDF(const Vec3 &pos, const Vec3 &dst, bool posOnLight) const
{
    return posOnLight ? geometry_->SamplePDF(pos) : 0.0;
}

bool GeometricDiffuseLight::IsDeltaPosition() const
{
    return false;
}

bool GeometricDiffuseLight::IsDeltaDirection() const
{
    return false;
}

bool GeometricDiffuseLight::IsDelta() const
{
    return false;
}

Spectrum GeometricDiffuseLight::AreaLe(const SurfacePoint &sp) const
{
    return radiance_;
}

Spectrum GeometricDiffuseLight::Le(const Ray &r) const
{
    return SPECTRUM::BLACK;
}

Spectrum GeometricDiffuseLight::Power() const
{
    return AGZ::Math::PI<float> * float(geometry_->SurfaceArea()) * radiance_;
}

AGZ_NS_END(Atrc)
