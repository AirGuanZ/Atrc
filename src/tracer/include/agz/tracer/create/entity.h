#pragma once

#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/geometry.h>
#include <agz/tracer/core/medium.h>

AGZ_TRACER_BEGIN

RC<Entity> create_diffuse_light(
    RC<const Geometry> geometry,
    const Spectrum &radiance,
    const MediumInterface &med,
    bool no_denoise);

RC<Entity> create_geometric(
    RC<const Geometry> geometry,
    RC<const Material> material,
    const MediumInterface &med,
    const Spectrum &emit_radiance,
    bool no_denoise);

AGZ_TRACER_END
