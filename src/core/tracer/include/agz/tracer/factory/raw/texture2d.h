#pragma once

#include <agz/tracer/core/texture2d.h>

AGZ_TRACER_BEGIN

std::shared_ptr<Texture2D> create_checker_board(
    const Texture2DCommonParams &common_params,
    int grid_count, const Spectrum &color1, const Spectrum &color2);

std::shared_ptr<Texture2D> create_constant_texture(
    const Texture2DCommonParams &common_params,
    const Spectrum &texel);

std::shared_ptr<Texture2D> create_constant_texture(
    const Texture2DCommonParams &common_params,
    real texel);

std::shared_ptr<Texture2D> create_gradient_texture(
    const Texture2DCommonParams &common_params,
    const Spectrum &color1, const Spectrum &color2);

std::shared_ptr<Texture2D> create_hdr_texture(
    const Texture2DCommonParams &common_params,
    const std::string &filename, const std::string &sampler);

std::shared_ptr<Texture2D> create_image_texture(
    const Texture2DCommonParams &common_params,
    const std::string &filename, const std::string &sampler);

std::shared_ptr<Texture2D> create_texture_scaler(
    const Texture2DCommonParams &common_params,
    const Spectrum &scale,
    std::shared_ptr<const Texture2D> internal);

std::shared_ptr<Texture2D> create_texture_adder(
    const Texture2DCommonParams &common_params,
    std::shared_ptr<const Texture2D> lhs,
    std::shared_ptr<const Texture2D> rhs);

std::shared_ptr<Texture2D> create_texture_multiplier(
    const Texture2DCommonParams &common_params,
    std::shared_ptr<const Texture2D> lhs,
    std::shared_ptr<const Texture2D> rhs);

std::shared_ptr<Texture2D> create_luminance_classifier(
    const Texture2DCommonParams &common_params,
    std::shared_ptr<const Texture2D> internal,
    std::shared_ptr<const Texture2D> thresholdTexture,
    std::shared_ptr<const Texture2D> higherTexture,
    std::shared_ptr<const Texture2D> lowerTexture);

std::shared_ptr<Texture2D> create_texture_reverse(
    const Texture2DCommonParams &common_params,
    std::shared_ptr<const Texture2D> internal);

AGZ_TRACER_END
