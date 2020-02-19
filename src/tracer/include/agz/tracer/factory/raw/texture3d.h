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

std::shared_ptr<Texture3D> create_texture3d_adder(
    const Texture3DCommonParams &common_params,
    std::shared_ptr<const Texture3D> lhs,
    std::shared_ptr<const Texture3D> rhs);

std::shared_ptr<Texture3D> create_texture3d_multiplier(
    const Texture3DCommonParams &common_params,
    std::shared_ptr<const Texture3D> lhs,
    std::shared_ptr<const Texture3D> rhs);

std::shared_ptr<Texture3D> create_texture3d_scaler(
    const Texture3DCommonParams &common_params,
    std::shared_ptr<const Texture3D> internal,
    const Spectrum &scale);

std::shared_ptr<Texture3D> create_texture3d_lum_classifier(
    const Texture3DCommonParams &common_params,
    std::shared_ptr<const Texture3D> lhs,
    std::shared_ptr<const Texture3D> rhs,
    std::shared_ptr<const Texture3D> less_or_equal,
    std::shared_ptr<const Texture3D> greater);

AGZ_TRACER_END
