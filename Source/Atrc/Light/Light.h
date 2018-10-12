#pragma once

#include <Atrc/Common.h>

AGZ_NS_BEG(Atrc)

struct LightSample
{
    Vec3r pos;
    Spectrum radiance;
    Real pdf;
};

ATRC_INTERFACE Light
{
public:

    virtual ~Light() = default;

    virtual Option<LightSample> SampleTo(const Intersection &inct) const = 0;

    virtual Spectrum Le(const Intersection &inct) const = 0;

    virtual Real Weight() const;
};

AGZ_NS_END(Atrc)
