#pragma once

#include <Atrc/Common.h>
#include <Atrc/Integrator/Integrator.h>

AGZ_NS_BEG(Atrc)

// See https://airguanz.github.io/2018/10/15/multiple-importance-sampling.html
class PathTracerMIS
    : ATRC_IMPLEMENTS Integrator,
      ATRC_PROPERTY AGZ::Uncopiable
{
public:

    explicit PathTracerMIS(int maxDepth);

    void SetBackground(const Spectrum &color);

    Spectrum GetRadiance(const Scene &scene, const Ray &r) const override;

private:

    int maxDepth_;
    Spectrum background_;

    Spectrum L(const Scene &scene, const Intersection &inct, int depth) const;

    Spectrum Ls(const Scene &scene, const Intersection &inct, int depth) const;

    Spectrum E(const Scene &scene, const Intersection &inct, const BxDF &bxdf) const;

    Spectrum E1(const Scene &scene, const Intersection &inct, const BxDF &bxdf) const;

    Spectrum E2(const Scene &scene, const Intersection &inct, const BxDF &bxdf) const;

    Spectrum S(const Scene &scene, const Intersection &inct, const BxDF &bxdf, int depth) const;
};

AGZ_NS_END(Atrc)
