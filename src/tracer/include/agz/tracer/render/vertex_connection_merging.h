#pragma once

#include <agz/tracer/core/light.h>
#include <agz/tracer/render/common.h>

AGZ_TRACER_RENDER_BEGIN

namespace vcm
{

struct RayPayload
{
    Spectrum accu_coef;
    int seg_cnt       = 0;
    bool on_light     = false;
    bool all_specular = false;

    real dVCM = 0;
    real dVC  = 0;
    real dVM  = 0;
};

struct Vertex
{
    Vec3 pos;
    Spectrum accu_coef;
    int seg_cnt      = 0;
    const BSDF *bsdf = nullptr;

    real dVCM = 0;
    real dVC  = 0;
    real dVM  = 0;
};



} // namespace vcm

AGZ_TRACER_RENDER_END
