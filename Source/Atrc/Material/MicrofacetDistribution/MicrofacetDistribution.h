#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

struct MicrofacetDistributionSampleResult
{
    Vec3 sample;
    Real wh;
};

class MicrofacetDistribution
{
public:

    virtual ~MicrofacetDistribution() = default;

    virtual Real Eval(const Vec3 &H) const = 0;

    virtual MicrofacetDistributionSampleResult SampleWh(const Vec3 &wo) const = 0;

    virtual Real SampleWhPDF(const Vec3 &wo, const Vec3 &wh) const = 0;
};

AGZ_NS_END(Atrc)
