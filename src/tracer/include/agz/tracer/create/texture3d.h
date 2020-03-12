#pragma once

#include <agz/tracer/core/texture3d.h>
#include <agz/utility/texture/texture3d.h>

AGZ_TRACER_BEGIN

RC<Texture3D> create_gray_grid_point3d(
    const Texture3DCommonParams &common_params,
    RC<const Image3D<real>> data);

RC<Texture3D> create_spectrum_grid_point3d(
    const Texture3DCommonParams &common_params,
    RC<const texture::texture3d_t<Spectrum>> data);

RC<Texture3D> create_constant3d_texture(
    const Texture3DCommonParams &common_params,
    const Spectrum &texel);

AGZ_TRACER_END
