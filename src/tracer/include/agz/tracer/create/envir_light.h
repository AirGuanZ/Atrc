#pragma once

#include <agz/tracer/core/light.h>
#include <agz/tracer/core/texture2d.h>

AGZ_TRACER_BEGIN

std::shared_ptr<EnvirLight> create_ibl_light(
    std::shared_ptr<const Texture2D> tex,
    bool no_importance_sampling = false);

std::shared_ptr<EnvirLight> create_native_sky(
    const Spectrum &top,
    const Spectrum &bottom);

AGZ_TRACER_END
