#pragma once

#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture2d.h>

AGZ_TRACER_BEGIN

std::shared_ptr<Material> create_disney_reflection(
    std::shared_ptr<const Texture2D> base_color,
    std::shared_ptr<const Texture2D> metallic,
    std::shared_ptr<const Texture2D> roughness,
    std::shared_ptr<const Texture2D> subsurface,
    std::shared_ptr<const Texture2D> specular,
    std::shared_ptr<const Texture2D> specular_tint,
    std::shared_ptr<const Texture2D> anisotropic,
    std::shared_ptr<const Texture2D> sheen,
    std::shared_ptr<const Texture2D> sheen_tint,
    std::shared_ptr<const Texture2D> clearcoat,
    std::shared_ptr<const Texture2D> clearcoat_gloss,
    std::unique_ptr<const NormalMapper> normal_mapper);

std::shared_ptr<Material> create_disney(
    std::shared_ptr<const Texture2D> base_color,
    std::shared_ptr<const Texture2D> metallic,
    std::shared_ptr<const Texture2D> roughness,
    std::shared_ptr<const Texture2D> transmission,
    std::shared_ptr<const Texture2D> transmission_roughness,
    std::shared_ptr<const Texture2D> ior,
    std::shared_ptr<const Texture2D> specular_scale,
    std::shared_ptr<const Texture2D> specular_tint,
    std::shared_ptr<const Texture2D> anisotropic,
    std::shared_ptr<const Texture2D> sheen,
    std::shared_ptr<const Texture2D> sheen_tint,
    std::shared_ptr<const Texture2D> clearcoat,
    std::shared_ptr<const Texture2D> clearcoat_gloss,
    std::unique_ptr<const NormalMapper> normal_mapper);

std::shared_ptr<Material> create_frosted_glass(
    std::shared_ptr<const Texture2D> color_map,
    std::shared_ptr<const Texture2D> roughness,
    std::shared_ptr<const Texture2D> ior);

std::shared_ptr<Material> create_glass(
    std::shared_ptr<const Texture2D> color_reflection_map,
    std::shared_ptr<const Texture2D> color_refraction_map,
    std::shared_ptr<const Texture2D> ior);

std::shared_ptr<Material> create_ideal_black();

std::shared_ptr<Material> create_ideal_diffuse(
    std::shared_ptr<const Texture2D> albedo,
    std::unique_ptr<const NormalMapper> normal_mapper);

std::shared_ptr<Material> create_invisible_surface();

std::shared_ptr<Material> create_mat_adder(
    std::vector<std::shared_ptr<const Material>> &&mats);

std::shared_ptr<Material> create_mat_scaler(
    std::shared_ptr<const Material> internal,
    std::shared_ptr<const Texture2D> scale);

std::shared_ptr<Material> create_mirror(
    std::shared_ptr<const Texture2D> color_map,
    std::shared_ptr<const Texture2D> eta,
    std::shared_ptr<const Texture2D> k);

std::shared_ptr<Material> create_mtl(
    std::shared_ptr<const Texture2D> kd,
    std::shared_ptr<const Texture2D> ks,
    std::shared_ptr<const Texture2D> ns);

std::shared_ptr<Material> create_mirror_varnish(
    std::shared_ptr<const Material> internal,
    std::shared_ptr<const Texture2D> eta_in,
    std::shared_ptr<const Texture2D> eta_out,
    std::shared_ptr<const Texture2D> color);

std::shared_ptr<Material> create_phong(
    std::shared_ptr<const Texture2D> d,
    std::shared_ptr<const Texture2D> s,
    std::shared_ptr<const Texture2D> ns,
    std::unique_ptr<NormalMapper> nor_map);

AGZ_TRACER_END
