#pragma once

#include "../Common.h"
#include "../Core/Integrator.h"
#include "../Sample/SampleSequence.h"

AGZ_NS_BEG(Atrc)

class WhittedRayTracer
    : ATRC_IMPLEMENTS Integrator,
      ATRC_PROPERTY AGZ::Uncopiable
{
    SampleSeq2D *lightSamSeq_;

    uint32_t lightSampleLevel_;
    uint32_t maxDepth_;

    Spectrum SingleLightIllumination(
        const Scene &scene, const Ray &r, const Light &light,
        const EntityIntersection &inct) const;

    Spectrum DirectIllumination(
        const Scene &scene, const Ray &r, const EntityIntersection &inct) const;

    Spectrum IndirectIllumination(
        const Scene &scene, const Ray &r, const EntityIntersection &inct,
        uint32_t depth) const;

    Spectrum Trace(const Scene &scene, const Ray &r, uint32_t depth) const;

public:

    explicit WhittedRayTracer(
        SampleSeq2D *lightSamSeq, uint32_t lightSampleLevel = 1, uint32_t maxDepth = 5);

    Spectrum GetRadiance(const Scene &scene, const Ray &r) const override;
};

AGZ_NS_END(Atrc)
