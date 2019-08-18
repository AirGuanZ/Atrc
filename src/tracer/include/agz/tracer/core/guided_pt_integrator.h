#pragma once

#include <agz/tracer/core/scene.h>
#include <agz/tracer/core/sampler.h>
#include <agz/tracer_utility/object.h>

AGZ_TRACER_BEGIN

namespace pgpt
{

    class Guider;
    class RecordBatchBuilder;

} // namespace pgpt

class GuidedPathTracingIntegrator : public obj::Object
{
public:

    using Object::Object;

    virtual Spectrum eval(const Scene &scene, const Ray &r, Sampler &sampler, Arena &arena,
        pgpt::RecordBatchBuilder &recorder, pgpt::Guider &guider, bool enable_trainer, bool enable_sampler) const = 0;
};

AGZT_INTERFACE(GuidedPathTracingIntegrator)

AGZ_TRACER_END
