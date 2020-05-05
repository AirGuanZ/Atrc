#pragma once

#include <agz/tracer/core/light.h>
#include <agz/tracer/render/common.h>

AGZ_TRACER_RENDER_BEGIN

namespace vcm
{

/*
VCM Algo:

    trace light subpaths and connect to camera
    build range search ds for the first iteration

    in each iteration:

        for each pixel:
            trace eye subpath, and for each eye vertex v:
                connect v to a light subpath
                perform range queries to merge with neighboring light vertices

        if current iteration isn't the last one:
            trace light subpaths and rebuild range search ds for next iteration

    interface:

        VCMRangeSearchAccelerator
            add_light_vertex
            find_in_neighborhood
        vcm_trace_light_subpath
        vcm_trace_eye_subpath
*/

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
