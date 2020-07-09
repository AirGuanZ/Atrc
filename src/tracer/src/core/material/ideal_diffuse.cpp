#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture2d.h>
#include <agz/utility/misc.h>

#include "./component/aggregate.h"
#include "./component/diffuse_comp.h"

AGZ_TRACER_BEGIN

class IdealDiffuse : public Material
{
    RC<const Texture2D> albedo_;
    Box<const NormalMapper> normal_mapper_;

public:

    IdealDiffuse(
        RC<const Texture2D> albedo,
        Box<const NormalMapper> normal_mapper)
    {
        albedo_ = albedo;
        normal_mapper_ = std::move(normal_mapper);
    }

    ShadingPoint shade(const EntityIntersection &inct, Arena &arena) const override
    {
        const FSpectrum albedo = albedo_->sample_spectrum(inct.uv);
        FCoord shading_coord = normal_mapper_->reorient(inct.uv, inct.user_coord);

        auto bsdf = arena.create<AggregateBSDF<1>>(
            inct.geometry_coord, shading_coord, albedo);
        bsdf->add_component(1, arena.create<DiffuseComponent>(albedo));

        ShadingPoint shd;
        shd.bsdf = bsdf;
        shd.shading_normal = shading_coord.z;

        return shd;
    }
};

RC<Material> create_ideal_diffuse(
    RC<const Texture2D> albedo,
    Box<const NormalMapper> normal_mapper)
{
    return newRC<IdealDiffuse>(albedo, std::move(normal_mapper));
}

AGZ_TRACER_END
