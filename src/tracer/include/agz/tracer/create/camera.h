#pragma once

#include <agz/tracer/core/camera.h>

AGZ_TRACER_BEGIN

RC<Camera> create_thin_lens_camera(
    real film_aspect,
    const FVec3 &pos,
    const FVec3 &dst,
    const FVec3 &up,
    real fov,
    real lens_radius,
    real focal_distance);

AGZ_TRACER_END
