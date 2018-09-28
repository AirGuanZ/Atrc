#pragma once

#include "../Common.h"
#include "../Core/Spectrum.h"
#include "../Math/AGZMath.h"
#include "../Sample/SampleSequence.h"

AGZ_NS_BEG(Atrc)

struct LightSample
{
    Spectrum spectrum;
    Vec3r pos;
    Vec3r dst2pos;
    Real pdf;
};

ATRC_INTERFACE Light
{
public:

    virtual ~Light() = default;

    virtual bool IsDeltaLight() const = 0;

    virtual LightSample SampleTo(const Vec3r &dst, SampleSeq2D &samSeq) const = 0;

    virtual uint32_t SampleCount(uint32_t sampleLevel) const = 0;
};

AGZ_NS_END(Atrc)
