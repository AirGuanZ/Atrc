#pragma once

#include <agz/tracer/core/light.h>
#include <agz/tracer/core/texture2d.h>

AGZ_TRACER_BEGIN

RC<EnvirLight> create_ibl_light(
    RC<const Texture2D> tex,
    bool no_importance_sampling = false);

RC<EnvirLight> create_native_sky(
    const Spectrum &top,
    const Spectrum &bottom);

AGZ_TRACER_END
