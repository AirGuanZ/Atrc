#pragma once

#include <agz/tracer/core/post_processor.h>

AGZ_TRACER_BEGIN

PostProcessor *create_aces_tone_mapper(
    real exposure,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

PostProcessor *create_film_flipper(
    bool vertically, bool horizontally,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

PostProcessor *create_gamma_corrector(
    real gamma,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

PostProcessor *create_oidn_denoiser(
    bool clamp_color,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

PostProcessor *create_saving_gbuffer_to_png(
    std::string albedo_filename,
    std::string normal_filename,
    std::string depth_filename,
    std::string binary_filename,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

PostProcessor *create_saving_to_img(
    std::string filename, std::string ext,
    bool open, real gamma, bool with_alpha_channel,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

AGZ_TRACER_END

