#pragma once

#include <agz/tracer/core/light.h>
#include <agz/tracer/core/texture.h>

AGZ_TRACER_BEGIN

std::shared_ptr<EnvirLight> create_directional_light(
    const Vec3 &dir,
    const Spectrum &radiance,
    real range);

std::shared_ptr<EnvirLight> create_hdri_light(
    std::shared_ptr<const Texture> tex,
    const Vec3 &up,
    real radius,
    const Vec3 &offset);

std::shared_ptr<EnvirLight> create_ibl_light(
    std::shared_ptr<const Texture> tex,
    const Vec3 &up);

std::shared_ptr<EnvirLight> create_native_sky(
    const Spectrum &top,
    const Spectrum &bottom,
    const Vec3 &up);

std::shared_ptr<EnvirLight> create_no_env();

AGZ_TRACER_END
