#pragma once

#include <Atrc/Common.h>
#include <Atrc/Integrator/Integrator.h>

AGZ_NS_BEG(Atrc)

// See https://airguanz.github.io/2018/10/12/direct-indirect-path-tracing.html
class PathTracerEx2
    : ATRC_IMPLEMENTS Integrator
{
    int lightSampleCount_;
    int maxDepth_;

    Spectrum background_;

    // returns L(inct.pos -> inct.wr)
    Spectrum L(const Ray &r, const Intersection &inct, const Scene &scene, int depth) const;

    // returns Ls(inct.pos -> inct.wr)
    Spectrum Ls(const Ray &r, const Intersection &inct, const Scene &scene, int depth) const;

    // returns E(inct.pos -> inct.wr)
    Spectrum E(const Ray &r, const Intersection &inct, const BxDF &bxdf, const Scene &scene) const;

    // returns S(inct.pos -> inct.wr)
    Spectrum S(const Ray &r, const Intersection &inct, const BxDF &bxdf, const Scene &scene, int depth) const;

public:

    PathTracerEx2(int lightSampleCount, int maxDepth);

    void SetBackground(const Spectrum &color);

    Spectrum GetRadiance(const Scene &scene, const Ray &r) const override;
};

AGZ_NS_END(Atrc)
