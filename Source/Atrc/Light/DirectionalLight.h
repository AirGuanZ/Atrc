#pragma once

#include <Atrc/Common.h>
#include <Atrc/Light/Light.h>
#include <Atrc/Math/Math.h>

AGZ_NS_BEG(Atrc)

class DirectionalLight
    : ATRC_IMPLEMENTS Light
{
    Spectrum radiance_;
    Vec3r dir_;

    Radr coverRad_;

    AABB world_;
    Real worldRadius_;

public:

    DirectionalLight(const Spectrum &radiance, const Vec3r &dir, Radr coverRad = Radr(0.0));

    void PreprocessScene(const Scene &scene) override;

    Option<LightSample> SampleTo(const Intersection &inct) const override;

    Spectrum Le(const Intersection &inct) const override;
};

AGZ_NS_END(Atrc)
