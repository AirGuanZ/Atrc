#pragma once

#include <agz/tracer/core/texture3d.h>
#include <agz-utils/texture/texture3d.h>

AGZ_TRACER_BEGIN

RC<Texture3D> create_constant3d_texture(
    const Texture3DCommonParams &common_params,
    const FSpectrum &texel);

RC<Texture3D> create_image3d(
    const Texture3DCommonParams &common_params,
    RC<const Image3D<real>> data,
    bool use_linear_sampler);

RC<Texture3D> create_image3d(
    const Texture3DCommonParams &common_params,
    RC<const Image3D<uint8_t>> data,
    bool use_linear_sampler);

RC<Texture3D> create_image3d(
    const Texture3DCommonParams &common_params,
    RC<const Image3D<Spectrum>> data,
    bool use_linear_sampler);

RC<Texture3D> create_image3d(
    const Texture3DCommonParams &common_params,
    RC<const Image3D<math::color3b>> data,
    bool use_linear_sampler);

AGZ_TRACER_END
