#pragma once

#include <agz/tracer/core/post_processor.h>

AGZ_TRACER_BEGIN

RC<PostProcessor> create_aces_tone_mapper(
    real exposure);

RC<PostProcessor> create_film_flipper(
    bool vertically, bool horizontally);

RC<PostProcessor> create_gamma_corrector(
    real gamma);

RC<PostProcessor> create_oidn_denoiser(
    bool clamp_color);

RC<PostProcessor> create_saving_gbuffer_to_png(
    std::string albedo_filename,
    std::string normal_filename);

RC<PostProcessor> create_saving_to_img(
    std::string filename, std::string ext,
    bool open, real gamma);

RC<PostProcessor> create_img_resizer(
    const Vec2i &target_size);

AGZ_TRACER_END
