#pragma once

#include <agz/tracer/core/post_processor.h>

AGZ_TRACER_BEGIN

std::shared_ptr<PostProcessor> create_aces_tone_mapper(
    real exposure);

std::shared_ptr<PostProcessor> create_film_flipper(
    bool vertically, bool horizontally);

std::shared_ptr<PostProcessor> create_gamma_corrector(
    real gamma);

std::shared_ptr<PostProcessor> create_oidn_denoiser(
    bool clamp_color);

std::shared_ptr<PostProcessor> create_saving_gbuffer_to_png(
    std::string albedo_filename,
    std::string normal_filename,
    std::string depth_filename,
    std::string binary_filename);

std::shared_ptr<PostProcessor> create_saving_to_img(
    std::string filename, std::string ext,
    bool open, real gamma, bool with_alpha_channel);

std::shared_ptr<PostProcessor> create_img_resizer(
    const Vec2i &target_size);

AGZ_TRACER_END
