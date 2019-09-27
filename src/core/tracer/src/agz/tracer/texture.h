#pragma once

#include <agz/tracer/core/texture.h>

AGZ_TRACER_BEGIN

Texture *create_checker_board(
    const TextureCommonParams &common_params,
    int grid_count, const Spectrum &color1, const Spectrum &color2,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

Texture *create_constant_texture(
    const TextureCommonParams &common_params,
    const Spectrum &texel,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

Texture *create_constant_texture(
    const TextureCommonParams &common_params,
    real texel,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

Texture *create_hdr_texture(
    const TextureCommonParams &common_params,
    const std::string &filename, const std::string &sampler,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

Texture *create_image_texture(
    const TextureCommonParams &common_params,
    const std::string &filename, const std::string &sampler,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

Texture *create_texture_scaler(
    const Spectrum &scale, const Texture *internal,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

Texture *create_gradient_texture(
    const TextureCommonParams &common_params,
    const Spectrum &color1, const Spectrum &color2,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

AGZ_TRACER_END
