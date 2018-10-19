#include <Atrc/Light/GeometricDiffuseLight.h>

AGZ_NS_BEG(Atrc)

GeometricDiffuseLight::GeometricDiffuseLight(const Geometry *geometry, const Spectrum &radiance)
    : geometry_(geometry), radiance_(radiance)
{
    AGZ_ASSERT(geometry);
}

LightSampleToResult GeometricDiffuseLight::SampleTo(const SurfacePoint &sp) const
{
    LightSampleToResult ret;
    GeometrySampleResult sam = geometry_->Sample();
    ret.pos = sam.pos;
    ret.nor = sam.nor;
    ret.pdf = sam.pdf;
    ret.le = radiance_;
    return ret;
}

Real GeometricDiffuseLight::SampleToPDF(const Vec3 &pos, const Vec3 &dst) const
{
    return geometry_->SamplePDF(pos);
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
