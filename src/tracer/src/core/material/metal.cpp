#include <agz/tracer/core/material.h>

#include "./utility/fresnel_point.h"
#include "./component/aggregate.h"
#include "./component/microfacet_refl_comp.h"

AGZ_TRACER_BEGIN

class Metal : public Material
{
    RC<const Texture2D> color_;

    RC<const Texture2D> k_;
    RC<const Texture2D> eta_;

    RC<const Texture2D> roughness_;
    RC<const Texture2D> anisotropic_;

    Box<NormalMapper> normal_mapper_;

public:

    Metal(
        RC<const Texture2D> color,
        RC<const Texture2D> k, RC<const Texture2D> eta,
        RC<const Texture2D> roughness, RC<const Texture2D> anisotropic,
        Box<NormalMapper> normal_mapper)
    {
        color_.swap(color);

        k_          .swap(k);
        eta_        .swap(eta);
        roughness_  .swap(roughness);
        anisotropic_.swap(anisotropic);

        normal_mapper_.swap(normal_mapper);
    }

    ShadingPoint shade(
        const EntityIntersection &inct, Arena &arena) const override
    {
        const Coord shading_coord = normal_mapper_->reorient(
            inct.uv, inct.user_coord);

        const Spectrum color   = color_->sample_spectrum(inct.uv);
        const Spectrum k       = k_->sample_spectrum(inct.uv);
        const Spectrum eta     = eta_->sample_spectrum(inct.uv);
        const real roughness   = roughness_->sample_real(inct.uv);
        const real anisotropic = anisotropic_->sample_real(inct.uv);

        const auto fresnel = arena.create<ColoredConductorPoint>(
            color, Spectrum(1), eta, k);

        auto *bsdf = arena.create<AggregateBSDF<1>>(
            inct.geometry_coord, shading_coord, color);
        bsdf->add_component(1, arena.create<GGXMicrofacetReflectionComponent>(
            fresnel, roughness, anisotropic));

        ShadingPoint shd;
        shd.bsdf = bsdf;
        shd.shading_normal = shading_coord.z;
        return shd;
    }
};

RC<Material> create_metal(
    RC<const Texture2D> color,
    RC<const Texture2D> eta,
    RC<const Texture2D> k,
    RC<const Texture2D> roughness,
    RC<const Texture2D> anisotropic,
    Box<NormalMapper> normal_mapper)
{
    return newRC<Metal>(
        std::move(color),
        std::move(eta), std::move(k),
        std::move(roughness), std::move(anisotropic),
        std::move(normal_mapper));
}

AGZ_TRACER_END
