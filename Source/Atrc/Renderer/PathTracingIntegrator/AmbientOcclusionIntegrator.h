#pragma once

#include <Atrc/Renderer/PathTracingIntegrator/PathTracingIntegrator.h>

AGZ_NS_BEG(Atrc)

class AmbientOcclusionIntegrator : public PathTracingIntegrator
{
    Real maxOccuT_;
    Spectrum bgdColor_;
    Spectrum objColor_;

public:

    explicit AmbientOcclusionIntegrator(Real maxOccuT, const Spectrum &backgroundColor, const Spectrum &objectColor);

    Spectrum Eval(const Scene &scene, const Ray &r, AGZ::ObjArena<> &arena) const override;
};

AGZ_NS_END(Atrc)
