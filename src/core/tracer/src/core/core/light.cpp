#include <agz/tracer/core/light.h>
#include <agz/tracer/core/scene.h>

AGZ_TRACER_BEGIN

void EnvirLight::preprocess(const AABB &world_bound)
{
    // 稍微放大world bound，避免数值精度问题导致某些点没落到包围盒中

    const auto [low, high] = world_bound;
    const real diag_len = (high - low).length();
    world_radius_ = real(1.1) * diag_len / 2;
    world_centre_ = real(0.5) * (low + high);
}

AGZ_TRACER_END
