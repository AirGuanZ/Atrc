#pragma once

#include <agz/tracer/core/fresnel.h>
#include <agz/tracer/core/texture.h>

AGZ_TRACER_BEGIN

Fresnel *create_always_one_fresnel(
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

Fresnel *create_conductor_fresnel(
    const Texture *eta_out,
    const Texture *eta_in,
    const Texture *k,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);
    
Fresnel *create_dielectric_fresnel(
    const Texture *eta_out,
    const Texture *eta_in,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

AGZ_TRACER_END
