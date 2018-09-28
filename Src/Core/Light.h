#pragma once

#include "../Common.h"
#include "../Core/Spectrum.h"
#include "../Math/AGZMath.h"

AGZ_NS_BEG(Atrc)

struct LightSample
{
    Spectrum spectrum;
    Vec3r wi;
    Real pdf;
};

ATRC_INTERFACE Light
{
public:

    virtual ~Light() = default;

    virtual bool IsDeltaLight() const = 0;

    virtual LightSample SampleTo(const Vec3r &dst) const = 0;

    virtual uint32_t SampleCount(uint32_t sampleLevel) const = 0;
};

AGZ_NS_END(Atrc)
