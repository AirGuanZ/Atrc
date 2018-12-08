#pragma once

#include <Atrc/Renderer/PathTracingIntegrator/PathTracingIntegrator.h>

AGZ_NS_BEG(Atrc)

class ShadingNormalIntegrator : public PathTracingIntegrator
{
public:

    Spectrum Eval(const Scene &scene, const Ray &r, AGZ::ObjArena<> &arena) const override;
};

AGZ_NS_END(Atrc)
