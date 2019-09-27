#pragma once

#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/geometry.h>

#include <agz/tracer/entity/medium_interface.h>

AGZ_TRACER_BEGIN

Entity *create_diffuse_light(
    const Geometry *geometry,
    const Spectrum &radiance,
    const MediumInterface &med,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

Entity *create_geometric(
    const Geometry *geometry,
    const Material *material,
    const MediumInterface &med,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

AGZ_TRACER_END
