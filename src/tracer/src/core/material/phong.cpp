#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture2d.h>
#include <agz/tracer/utility/reflection.h>

#include "./component/aggregate.h"
#include "./component/diffuse_comp.h"
#include "./component/phong_specular_comp.h"

AGZ_TRACER_BEGIN

class Phong : public Material
{
    RC<const Texture2D> d_;
    RC<const Texture2D> s_;
    RC<const Texture2D> ns_;

    Box<NormalMapper> nor_map_;

public:

    Phong(
        RC<const Texture2D> d,
        RC<const Texture2D> s,
        RC<const Texture2D> ns,
        Box<NormalMapper> nor_map)
        : d_(std::move(d)), s_(std::move(s)),
          ns_(std::move(ns)), nor_map_(std::move(nor_map))
    {
        
    }

    ShadingPoint shade(const EntityIntersection &inct, Arena &arena) const override
    {
        Spectrum d = d_->sample_spectrum(inct.uv);
        Spectrum s = s_->sample_spectrum(inct.uv);
        const real ns = ns_->sample_real(inct.uv);

        // ensure energy conservation

        real dem = 1;
        for(int i = 0; i < SPECTRUM_COMPONENT_COUNT; ++i)
            dem = (std::max)(dem, d[i] + s[i]);
        d /= dem;
        s /= dem;

        // shading coord

        const Coord shading_coord = nor_map_->reorient(inct.uv, inct.user_coord);

        // compute diffuse/specular weight

        const real diffuse_lum  = d.lum();
        const real specular_lum = s.lum();

        real diffuse_weight;
        if(diffuse_lum + specular_lum > EPS())
            diffuse_weight = diffuse_lum / (diffuse_lum + specular_lum);
        else
            diffuse_weight = real(0.5);

        const real specular_weight = 1 - diffuse_weight;

        // construct bsdf

        auto bsdf = arena.create<AggregateBSDF<2>>(
            inct.geometry_coord, shading_coord, d + s);
        bsdf->add_component(
            diffuse_weight, arena.create<DiffuseComponent>(d));
        bsdf->add_component(
            specular_weight, arena.create<PhongSpecularComponent>(s, ns));

        ShadingPoint shd;
        shd.bsdf = bsdf;
        shd.shading_normal = shading_coord.z;

        return shd;
    }
};

RC<Material> create_phong(
    RC<const Texture2D> d,
    RC<const Texture2D> s,
    RC<const Texture2D> ns,
    Box<NormalMapper> nor_map)
{
    return newRC<Phong>(
        std::move(d), std::move(s), std::move(ns), std::move(nor_map));
}

AGZ_TRACER_END
