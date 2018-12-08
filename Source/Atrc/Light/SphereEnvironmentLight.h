#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class SphereEnvironmentLight : public Light
{
    const Texture *envTex_;

    Transform local2World_;

    Vec3 worldCentre_;
    Real worldRadius_;

public:

    SphereEnvironmentLight(const Texture *envTex, const Transform &local2World);

    void PreprocessScene(const Scene &scene) override;

    LightSampleToResult SampleLi(const SurfacePoint &sp) const override;

    Real SampleLiPDF(const Vec3 &pos, const SurfacePoint &dst, bool posOnLight) const override;

    bool IsDeltaPosition() const override;

    bool IsDeltaDirection() const override;

    bool IsDelta() const override;

    Spectrum AreaLe(const SurfacePoint &sp) const override;

    bool IgnoreFirstMedium() const override;

    Spectrum NonareaLe(const Ray &r) const override;

    Spectrum Power() const override;
};

AGZ_NS_END(Atrc)
