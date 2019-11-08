#pragma once

#include <agz/tracer/core/texture.h>

AGZ_TRACER_BEGIN

std::shared_ptr<Texture> create_checker_board(
    const TextureCommonParams &common_params,
    int grid_count, const Spectrum &color1, const Spectrum &color2);

std::shared_ptr<Texture> create_constant_texture(
    const TextureCommonParams &common_params,
    const Spectrum &texel);

std::shared_ptr<Texture> create_constant_texture(
    const TextureCommonParams &common_params,
    real texel);

std::shared_ptr<Texture> create_gradient_texture(
    const TextureCommonParams &common_params,
    const Spectrum &color1, const Spectrum &color2);

std::shared_ptr<Texture> create_hdr_texture(
    const TextureCommonParams &common_params,
    const std::string &filename, const std::string &sampler);

std::shared_ptr<Texture> create_image_texture(
    const TextureCommonParams &common_params,
    const std::string &filename, const std::string &sampler);

std::shared_ptr<Texture> create_texture_scaler(
    const TextureCommonParams &common_params,
    const Spectrum &scale,
    std::shared_ptr<const Texture> internal);

std::shared_ptr<Texture> create_texture_adder(
    const TextureCommonParams &common_params,
    std::shared_ptr<const Texture> lhs,
    std::shared_ptr<const Texture> rhs);

std::shared_ptr<Texture> create_texture_multiplier(
    const TextureCommonParams &common_params,
    std::shared_ptr<const Texture> lhs,
    std::shared_ptr<const Texture> rhs);

std::shared_ptr<Texture> create_luminance_classifier(
    const TextureCommonParams &common_params,
    std::shared_ptr<const Texture> internal,
    std::shared_ptr<const Texture> thresholdTexture,
    std::shared_ptr<const Texture> higherTexture,
    std::shared_ptr<const Texture> lowerTexture);

std::shared_ptr<Texture> create_texture_reverse(
    const TextureCommonParams &common_params,
    std::shared_ptr<const Texture> internal);

AGZ_TRACER_END
