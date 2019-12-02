#pragma once

#include <agz/tracer/core/fresnel.h>
#include <agz/tracer/core/texture2d.h>

AGZ_TRACER_BEGIN

std::shared_ptr<Fresnel> create_always_one_fresnel();

std::shared_ptr<Fresnel> create_conductor_fresnel(
    std::shared_ptr<const Texture2D> eta_out,
    std::shared_ptr<const Texture2D> eta_in,
    std::shared_ptr<const Texture2D> k);
    
std::shared_ptr<Fresnel> create_dielectric_fresnel(
    std::shared_ptr<const Texture2D> eta_out,
    std::shared_ptr<const Texture2D> eta_in);

AGZ_TRACER_END
