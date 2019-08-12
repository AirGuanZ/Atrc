#pragma once

#include <agz/tracer/core/renderer.h>
#include <agz/tracer/core/sampler.h>

AGZ_TRACER_BEGIN

/**
 * @brief 用于PathTracer的Integrator
 */
class PathTracingIntegrator : public obj::Object
{
public:

    using Object::Object;

    /**
     * @brief 采样scene中沿-r.d的radiance
     * 
     * 返回的第二个分量表示是否击中了任何物体
     */
    virtual Spectrum eval(GBufferPixel *gpixel, const Scene &scene, const Ray &r, Sampler &sampler, Arena &arena) const = 0;
};

AGZT_INTERFACE(PathTracingIntegrator)

AGZ_TRACER_END
