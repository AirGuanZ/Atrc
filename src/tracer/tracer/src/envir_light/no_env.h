#pragma once

#include <agz/tracer/core/envir_light.h>

AGZ_TRACER_BEGIN

class NoEnv : public EnvironmentLight
{
public:

    EnvironmentLightSampleResult sample(const Sample3 &sam) const noexcept override
    {
        return {};
    }

    real pdf(const Vec3 &) const noexcept override
    {
        return 0;
    }

    Spectrum power() const noexcept override
    {
        return {};
    }

    void preprocess(const Scene &) override
    {
        
    }

    Spectrum radiance(const Vec3 &) const noexcept override
    {
        return {};
    }
};

AGZ_TRACER_END
