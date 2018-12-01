#pragma once

#include <Atrc/Renderer/PathTracingIntegrator/PathTracingIntegrator.h>

AGZ_NS_BEG(Atrc)

class PureColorIntegrator : public PathTracingIntegrator
{
    Spectrum background_;
    Spectrum entity_;

public:

    PureColorIntegrator(const Spectrum &background, const Spectrum &entity);

    Spectrum Eval(const Scene &scene, const Ray &r, AGZ::ObjArena<> &arena) const override;
};

AGZ_NS_END(Atrc)
