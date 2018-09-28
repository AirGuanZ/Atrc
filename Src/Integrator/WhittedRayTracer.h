#pragma once

#include "../Common.h"
#include "../Core/Integrator.h"

AGZ_NS_BEG(Atrc)

class WhittedRayTracer
    : ATRC_IMPLEMENTS Integrator,
      ATRC_PROPERTY AGZ::Uncopiable
{
    uint32_t maxDepth_;

    Spectrum Trace(const Scene &scene, const Ray &r, uint32_t depth) const;

public:

    explicit WhittedRayTracer(uint32_t maxDepth = 5);

    Spectrum GetRadiance(const Scene &scene, const Ray &r) const override;
};

AGZ_NS_END(Atrc)
