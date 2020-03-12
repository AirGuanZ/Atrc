#pragma once

#include <agz/tracer/common.h>

#define AGZ_TRACER_RENDER_BEGIN AGZ_TRACER_BEGIN namespace render {
#define AGZ_TRACER_RENDER_END   } AGZ_TRACER_END

AGZ_TRACER_RENDER_BEGIN

struct GBufferPixel
{
    Spectrum albedo;
    Vec3 normal;
    real denoise = 1;
};

struct Pixel : GBufferPixel
{
    Spectrum value;
};

AGZ_TRACER_RENDER_END
