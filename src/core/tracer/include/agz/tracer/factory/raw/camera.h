#pragma once

#include <agz/tracer/core/camera.h>

AGZ_TRACER_BEGIN

std::shared_ptr<Camera> create_pinhole(
    const Vec3 &pos,
    const Vec3 &dst,
    const Vec3 &up,
    real sensor_width,
    real sensor_aspect,
    real dist);

AGZ_TRACER_END
