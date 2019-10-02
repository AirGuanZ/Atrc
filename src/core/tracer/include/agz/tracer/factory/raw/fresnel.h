#pragma once

#include <agz/tracer/core/fresnel.h>
#include <agz/tracer/core/texture.h>

AGZ_TRACER_BEGIN

std::shared_ptr<Fresnel> create_always_one_fresnel();

std::shared_ptr<Fresnel> create_conductor_fresnel(
    std::shared_ptr<const Texture> eta_out,
    std::shared_ptr<const Texture> eta_in,
    std::shared_ptr<const Texture> k);
    
std::shared_ptr<Fresnel> create_dielectric_fresnel(
    std::shared_ptr<const Texture> eta_out,
    std::shared_ptr<const Texture> eta_in);

AGZ_TRACER_END
