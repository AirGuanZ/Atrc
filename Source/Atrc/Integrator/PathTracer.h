#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class PathTracer : public Integrator
{
public:

    explicit PathTracer(int maxDepth);

    Spectrum Eval(const Scene &scene, const Ray &r, AGZ::ObjArena<> &arena) const override;

private:

    Spectrum L(const Scene &scene, const SurfacePoint &sp, int depth, AGZ::ObjArena<> &arena) const;
    
    Spectrum Ls(const Scene &scene, const SurfacePoint &sp, int depth, AGZ::ObjArena<> &arena) const;

    Spectrum E(const Scene &scene, const SurfacePoint &sp, const ShadingPoint &shd) const;

    Spectrum E1(const Scene &scene, const SurfacePoint &sp, const ShadingPoint &shd) const;
    Spectrum E2(const Scene &scene, const SurfacePoint &sp, const ShadingPoint &shd) const;

    Spectrum S(const Scene &scene, const SurfacePoint &sp, const ShadingPoint &shd, int depth, AGZ::ObjArena<> &arena) const;

    int maxDepth_;
    Spectrum background_;
};

AGZ_NS_END(Atrc)
