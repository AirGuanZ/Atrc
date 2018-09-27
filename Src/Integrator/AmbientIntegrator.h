#pragma once

#include "../Common.h"
#include "../Core/Integrator.h"

AGZ_NS_BEG(Atrc)

class AmbientIntegrator
    : ATRC_IMPLEMENTS Integrator,
      ATRC_PROPERTY AGZ::Uncopiable
{
public:

    Spectrum GetRadiance(const Scene &scene, const Ray &r) const override;
};

AGZ_NS_END(Atrc)
