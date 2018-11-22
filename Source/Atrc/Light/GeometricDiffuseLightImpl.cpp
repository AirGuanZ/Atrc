#include <Atrc/Light/GeometricDiffuseLightImpl.h>

AGZ_NS_BEG(Atrc)

GeometricDiffuseLightImpl::GeometricDiffuseLightImpl(const Geometry *geometry, const Spectrum &radiance)
    : GeometricLight(geometry), radiance_(radiance)
{
    AGZ_ASSERT(geometry);
}

LightSampleToResult GeometricDiffuseLightImpl::SampleLi(const SurfacePoint &sp) const
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

Real GeometricDiffuseLightImpl::SampleLiPDF(const Vec3 &pos, const SurfacePoint &dst, bool posOnLight) const
{
    return posOnLight ? geometry_->SamplePDF(pos, dst.pos) : Real(0);
}

bool GeometricDiffuseLightImpl::IsDeltaPosition() const
{
    return false;
}

bool GeometricDiffuseLightImpl::IsDeltaDirection() const
{
    return false;
}

bool GeometricDiffuseLightImpl::IsDelta() const
{
    return false;
}

Spectrum GeometricDiffuseLightImpl::AreaLe([[maybe_unused]] const SurfacePoint &sp) const
{
    return radiance_;
}

Spectrum GeometricDiffuseLightImpl::NonareaLe([[maybe_unused]] const Ray &r) const
{
    return SPECTRUM::BLACK;
}

Spectrum GeometricDiffuseLightImpl::Power() const
{
    return AGZ::Math::PI<float> * float(geometry_->SurfaceArea()) * radiance_;
}

AGZ_NS_END(Atrc)
