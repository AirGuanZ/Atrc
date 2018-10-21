#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class PathTracer : public Integrator
{
public:

    explicit PathTracer(int maxDepth, const Spectrum &background = Spectrum());

    void SetBackground(const Spectrum &background);

    Spectrum GetRadiance(const Scene &scene, const Ray &r) const override;

private:

    Spectrum L(const Scene &scene, const SurfacePoint &sp, int depth) const;
    
    Spectrum Ls(const Scene &scene, const SurfacePoint &sp, int depth) const;

    Spectrum E(const Scene &scene, const SurfacePoint &sp, const ShadingPoint &shd) const;

    Spectrum E1(const Scene &scene, const SurfacePoint &sp, const ShadingPoint &shd) const;
    Spectrum E2(const Scene &scene, const SurfacePoint &sp, const ShadingPoint &shd) const;

    Spectrum S(const Scene &scene, const SurfacePoint &sp, const ShadingPoint &shd, int depth) const;

    int maxDepth_;
    Spectrum background_;
};

AGZ_NS_END(Atrc)
