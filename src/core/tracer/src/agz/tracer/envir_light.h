#pragma once

#include <agz/tracer/core/light.h>
#include <agz/tracer/core/texture.h>

AGZ_TRACER_BEGIN

EnvirLight *create_directional_light(
    const Vec3 &dir,
    const Spectrum &radiance,
    real range,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

EnvirLight *create_hdri_light(
    const Texture *tex,
    const Vec3 &up,
    real radius,
    const Vec3 &offset,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

EnvirLight *create_ibl_light(
    const Texture *tex,
    const Vec3 &up,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

EnvirLight *create_native_sky(
    const Spectrum &top,
    const Spectrum &bottom,
    const Vec3 &up,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

AGZ_TRACER_END
