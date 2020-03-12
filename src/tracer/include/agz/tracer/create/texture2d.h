#pragma once

#include <agz/tracer/core/texture2d.h>

AGZ_TRACER_BEGIN

RC<Texture2D> create_checker_board(
    const Texture2DCommonParams &common_params,
    int grid_count, const Spectrum &color1, const Spectrum &color2);

RC<Texture2D> create_constant2d_texture(
    const Texture2DCommonParams &common_params,
    const Spectrum &texel);

RC<Texture2D> create_constant2d_texture(
    const Texture2DCommonParams &common_params,
    real texel);

RC<Texture2D> create_hdr_texture(
    const Texture2DCommonParams &common_params,
    RC<const Image2D<math::color3f>> data, const std::string &sampler);

RC<Texture2D> create_image_texture(
    const Texture2DCommonParams &common_params,
    RC<const Image2D<math::color3b>> data, const std::string &sampler);

AGZ_TRACER_END
