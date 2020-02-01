#pragma once

#include <agz/tracer/core/camera.h>

AGZ_TRACER_BEGIN

std::shared_ptr<Camera> create_thin_lens_camera(
    real film_aspect,
    const Vec3 &pos,
    const Vec3 &dst,
    const Vec3 &up,
    real fov,
    real lens_radius,
    real focal_distance);

AGZ_TRACER_END
