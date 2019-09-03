#pragma once

#include <agz/tracer_utility/math.h>
#include <agz/tracer_utility/object.h>

AGZ_TRACER_BEGIN

class Scene;

struct EnvironmentLightSampleResult
{
    Vec3 ref_to_light;
    Spectrum radiance;
    real pdf      = 0;
    bool is_delta = false;

    bool valid() const noexcept
    {
        return !!ref_to_light;
    }
};

class EnvironmentLight : public obj::Object
{
protected:

    real world_radius_ = 1;
    Vec3 world_centre_;

public:

    using Object::Object;

    virtual EnvironmentLightSampleResult sample(const Sample3 &sam) const noexcept = 0;

    virtual real pdf(const Vec3 &ref_to_light) const noexcept = 0;

    virtual Spectrum power() const noexcept = 0;

    virtual void preprocess(const Scene &scene);

    virtual Spectrum radiance(const Vec3 &ref_to_light) const noexcept = 0;
};

AGZT_INTERFACE(EnvironmentLight)

AGZ_TRACER_END
