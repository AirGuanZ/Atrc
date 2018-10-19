#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class DirectIlluminationIntegrator : public Integrator
{
    Spectrum background_;

    Spectrum IlluminationBetween(
        const Scene &scene, const Light &light, const SurfacePoint &sp, const ShadingPoint &shd) const;

public:

    explicit DirectIlluminationIntegrator(const Spectrum &background);

    Spectrum GetRadiance(const Scene &scene, const Ray &r) const override;
};

AGZ_NS_END(Atrc)
