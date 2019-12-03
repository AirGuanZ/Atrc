#pragma once

#include <agz/tracer/core/texture3d.h>
#include <agz/utility/texture/texture3d.h>

AGZ_TRACER_BEGIN

std::shared_ptr<Texture3D> create_gray_grid_point3d(
    const Texture3DCommonParams &common_params, texture::texture3d_t<real> data);

std::shared_ptr<Texture3D> create_spectrum_grid_point3d(
    const Texture3DCommonParams &common_params, texture::texture3d_t<Spectrum> data);

std::shared_ptr<Texture3D> create_constant3d_texture(
    const Texture3DCommonParams &common_params,
    const Spectrum &texel);

AGZ_TRACER_END
