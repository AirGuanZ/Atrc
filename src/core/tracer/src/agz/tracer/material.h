#pragma once

#include <agz/tracer/core/fresnel.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture.h>

#include <agz/tracer/material/utility/normal_mapper.h>

AGZ_TRACER_BEGIN

Material *create_disney_reflection(
    const Texture *base_color,
    const Texture *metallic,
    const Texture *roughness,
    const Texture *subsurface,
    const Texture *specular,
    const Texture *specular_tint,
    const Texture *anisotropic,
    const Texture *sheen,
    const Texture *sheen_tint,
    const Texture *clearcoat,
    const Texture *clearcoat_gloss,
    const NormalMapper *normal_mapper,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

Material *create_disney(
    const Texture *base_color,
    const Texture *metallic,
    const Texture *roughness,
    const Texture *transmission,
    const Texture *transmission_roughness,
    const Texture *ior,
    const Texture *specular_tint,
    const Texture *anisotropic,
    const Texture *sheen,
    const Texture *sheen_tint,
    const Texture *clearcoat,
    const Texture *clearcoat_gloss,
    const Texture *scatter_distance,
    const NormalMapper *normal_mapper,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

Material *create_frosted_glass(
    const Texture *color_map,
    const Texture *roughness,
    const Fresnel *fresnel,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

Material *create_glass(
    const Texture *color_map,
    const Fresnel *fresnel,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

Material *create_ideal_black(
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

Material *create_ideal_diffuse(
    const Texture *albedo,
    const NormalMapper *normal_mapper,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

Material *create_mat_adder(
    std::vector<const Material*> &&mats,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

Material *create_mat_scaler(
    const Material *internal,
    const Texture *scale,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

Material *create_mirror(
    const Texture *color_map,
    const Fresnel *fresnel,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

Material *create_mtl(
    const Texture *kd,
    const Texture *ks,
    const Texture *ns,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

Material *create_subsurface(
    const Material *surface,
    const Texture *d,
    const Texture *A,
    const Texture *ior,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

Material *create_mirror_varnish(
    const Material *internal,
    const Texture *eta_in,
    const Texture *eta_out,
    const Texture *color,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

AGZ_TRACER_END
