#pragma once

#include <agz/tracer/core/fresnel.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture.h>

AGZ_TRACER_BEGIN

std::shared_ptr<Material> create_disney_reflection(
    std::shared_ptr<const Texture> base_color,
    std::shared_ptr<const Texture> metallic,
    std::shared_ptr<const Texture> roughness,
    std::shared_ptr<const Texture> subsurface,
    std::shared_ptr<const Texture> specular,
    std::shared_ptr<const Texture> specular_tint,
    std::shared_ptr<const Texture> anisotropic,
    std::shared_ptr<const Texture> sheen,
    std::shared_ptr<const Texture> sheen_tint,
    std::shared_ptr<const Texture> clearcoat,
    std::shared_ptr<const Texture> clearcoat_gloss,
    std::unique_ptr<const NormalMapper> normal_mapper);

std::shared_ptr<Material> create_disney(
    std::shared_ptr<const Texture> base_color,
    std::shared_ptr<const Texture> metallic,
    std::shared_ptr<const Texture> roughness,
    std::shared_ptr<const Texture> transmission,
    std::shared_ptr<const Texture> transmission_roughness,
    std::shared_ptr<const Texture> ior,
    std::shared_ptr<const Texture> specular_scale,
    std::shared_ptr<const Texture> specular_tint,
    std::shared_ptr<const Texture> anisotropic,
    std::shared_ptr<const Texture> sheen,
    std::shared_ptr<const Texture> sheen_tint,
    std::shared_ptr<const Texture> clearcoat,
    std::shared_ptr<const Texture> clearcoat_gloss,
    std::unique_ptr<const NormalMapper> normal_mapper);

std::shared_ptr<Material> create_frosted_glass(
    std::shared_ptr<const Texture> color_map,
    std::shared_ptr<const Texture> roughness,
    std::shared_ptr<const Fresnel> fresnel);

std::shared_ptr<Material> create_glass(
    std::shared_ptr<const Texture> color_reflection_map,
    std::shared_ptr<const Texture> color_refraction_map,
    std::shared_ptr<const Fresnel> fresnel);

std::shared_ptr<Material> create_ideal_black();

std::shared_ptr<Material> create_ideal_diffuse(
    std::shared_ptr<const Texture> albedo,
    std::unique_ptr<const NormalMapper> normal_mapper);

std::shared_ptr<Material> create_mat_adder(
    std::vector<std::shared_ptr<const Material>> &&mats);

std::shared_ptr<Material> create_mat_scaler(
    std::shared_ptr<const Material> internal,
    std::shared_ptr<const Texture> scale);

std::shared_ptr<Material> create_mirror(
    std::shared_ptr<const Texture> color_map,
    std::shared_ptr<const Fresnel> fresnel);

std::shared_ptr<Material> create_mtl(
    std::shared_ptr<const Texture> kd,
    std::shared_ptr<const Texture> ks,
    std::shared_ptr<const Texture> ns);

std::shared_ptr<Material> create_mirror_varnish(
    std::shared_ptr<const Material> internal,
    std::shared_ptr<const Texture> eta_in,
    std::shared_ptr<const Texture> eta_out,
    std::shared_ptr<const Texture> color);

AGZ_TRACER_END
