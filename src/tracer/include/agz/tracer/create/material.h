#pragma once

#include <agz/tracer/core/bssrdf.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture2d.h>

AGZ_TRACER_BEGIN

RC<Material> create_disney(
    RC<const Texture2D> base_color,
    RC<const Texture2D> metallic,
    RC<const Texture2D> roughness,
    RC<const Texture2D> transmission,
    RC<const Texture2D> transmission_roughness,
    RC<const Texture2D> ior,
    RC<const Texture2D> specular_scale,
    RC<const Texture2D> specular_tint,
    RC<const Texture2D> anisotropic,
    RC<const Texture2D> sheen,
    RC<const Texture2D> sheen_tint,
    RC<const Texture2D> clearcoat,
    RC<const Texture2D> clearcoat_gloss,
    Box<const NormalMapper> normal_mapper,
    RC<const BSSRDFSurface> bssrdf);

RC<Material> create_dream_works_fabric(
    RC<const Texture2D> color,
    RC<const Texture2D> roughness,
    Box<const NormalMapper> normal_mapper);

RC<Material> create_glass(
    RC<const Texture2D> color_reflection_map,
    RC<const Texture2D> color_refraction_map,
    RC<const Texture2D> ior,
    RC<const BSSRDFSurface> bssrdf);

RC<Material> create_ideal_black();

RC<Material> create_ideal_diffuse(
    RC<const Texture2D> albedo,
    Box<const NormalMapper> normal_mapper);

RC<Material> create_invisible_surface(
    RC<const BSSRDFSurface> bssrdf);

RC<Material> create_metal(
    RC<const Texture2D> color,
    RC<const Texture2D> eta,
    RC<const Texture2D> k,
    RC<const Texture2D> roughness,
    RC<const Texture2D> anisotropic,
    Box<NormalMapper> normal_mapper);

RC<Material> create_mirror(
    RC<const Texture2D> color_map,
    RC<const Texture2D> eta,
    RC<const Texture2D> k);

RC<Material> create_paper(
    RC<const Texture2D> color,
    real gf, real gb, real wf, real wb,
    real front_eta, real back_eta,
    real d, real sigma_s, real sigma_a,
    real front_roughness, real back_roughness,
    Box<const NormalMapper> normal_mapper);

RC<Material> create_phong(
    RC<const Texture2D> d,
    RC<const Texture2D> s,
    RC<const Texture2D> ns,
    Box<NormalMapper> nor_map);

RC<BSSRDFSurface> create_normalized_diffusion_bssrdf_surface(
    RC<const Texture2D> A,
    RC<const Texture2D> dmfp,
    RC<const Texture2D> eta);

AGZ_TRACER_END
