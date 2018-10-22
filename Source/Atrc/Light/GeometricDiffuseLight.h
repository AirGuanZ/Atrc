#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class GeometricDiffuseLight : public GeometricLight
{
    Spectrum radiance_;

public:

    GeometricDiffuseLight(const Geometry *geometry, const Spectrum &radiance);

    LightSampleToResult SampleLi(const SurfacePoint &sp) const override;

    Real SampleLiPDF(const Vec3 &pos, const Vec3 &dst, bool posOnLight) const override;

    bool IsDeltaPosition() const override;

    bool IsDeltaDirection() const override;

    bool IsDelta() const override;

    Spectrum AreaLe(const SurfacePoint &sp) const override;

    Spectrum NonareaLe(const Ray &r) const override;

    Spectrum Power() const override;
};

AGZ_NS_END(Atrc)
