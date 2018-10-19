#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class PureColorIntegrator : public Integrator
{
    Spectrum background_;
    Spectrum entity_;

public:

    PureColorIntegrator(const Spectrum &background, const Spectrum &entity);

    Spectrum GetRadiance(const Scene &scene, const Ray &r) const override;
};

AGZ_NS_END(Atrc)
