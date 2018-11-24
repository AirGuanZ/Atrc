#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class SHLightProjector : public Integrator
{
    Real(*sh_)(const Vec3&);
    int N_;

public:

    SHLightProjector(int l, int m, int N);

    Spectrum Eval(const Scene &scene, const Ray &r, AGZ::ObjArena<> &arena) const override;
};

AGZ_NS_END(Atrc)
