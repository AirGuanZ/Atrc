#pragma once

#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/geometry.h>
#include <agz/tracer/core/medium.h>

AGZ_TRACER_BEGIN

std::shared_ptr<Entity> create_diffuse_light(
    std::shared_ptr<const Geometry> geometry,
    const Spectrum &radiance,
    const MediumInterface &med,
    bool no_denoise);

std::shared_ptr<Entity> create_geometric(
    std::shared_ptr<const Geometry> geometry,
    std::shared_ptr<const Material> material,
    const MediumInterface &med,
    bool no_denoise);

AGZ_TRACER_END
