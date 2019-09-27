#pragma once

#include <agz/tracer/core/medium.h>

AGZ_TRACER_BEGIN

Medium *create_absorbtion_medium(
    const Spectrum &sigma_a,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

Medium *create_homogeneous_medium(
    const Spectrum &sigma_a,
    const Spectrum &sigma_s,
    real g,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

Medium *create_void(
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

AGZ_TRACER_END

