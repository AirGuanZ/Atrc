#pragma once

#include <Atrc/Common.h>
#include <Atrc/Integrator/Integrator.h>

AGZ_NS_BEG(Atrc)

class PathTracerEx
    : ATRC_IMPLEMENTS Integrator,
      ATRC_PROPERTY AGZ::Uncopiable
{
    int lightSampleCount_;
    int maxDepth_;

    Spectrum background_;

    Spectrum L(const Ray &r, const Intersection &inct, const Scene &scene, int depth) const;
    Spectrum E(const Ray &r, const Intersection &inct, const BxDF &bxdf, const Scene &scene) const;
    Spectrum S(const Ray &r, const Intersection &inct, const BxDF &bxdf, const Scene &scene, int depth) const;

public:

    PathTracerEx(int lightSampleCount, int maxDepth);

    void SetBackground(const Spectrum &color);

    Spectrum GetRadiance(const Scene &scene, const Ray &r) const override;
};

AGZ_NS_END(Atrc)
