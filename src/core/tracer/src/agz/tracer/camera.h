#pragma once

#include <agz/tracer/core/camera.h>

AGZ_TRACER_BEGIN

Camera *create_pinhole(
    const Vec3 &pos,
    const Vec3 &dst,
    const Vec3 &up,
    real sensor_width,
    real sensor_aspect,
    real dist,
    obj::Object::CustomedFlag customed_flag,
    Arena arena);

AGZ_TRACER_END
