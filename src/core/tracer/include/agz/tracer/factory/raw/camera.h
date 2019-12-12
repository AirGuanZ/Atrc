#pragma once

#include <agz/tracer/core/camera.h>

AGZ_TRACER_BEGIN

std::shared_ptr<Camera> create_thin_lens_camera(
    std::shared_ptr<const Film> film,
    const Vec3 &pos,
    const Vec3 &dst,
    const Vec3 &up,
    real fov,
    real aspect,
    real lens_radius,
    real focal_distance);

AGZ_TRACER_END
