#include <agz/tracer/core/bssrdf.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture.h>

#include "./bssrdf/constant_bssrdf.h"

AGZ_TRACER_BEGIN

class Subsurface : public Material
{
    Material *surface_ = nullptr;
    Texture  *d_       = nullptr;
    Texture  *A_       = nullptr;
    Texture  *eta_     = nullptr;

public:

    using Material::Material;

    static std::string description()
    {
        return R"___(
subsurface [Material]
    surface [Material] surface material
    d       [Texture]  mean free path length
    A       [Texture]  surface albedo
    ior     [Texture]  ior
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        init_customed_flag(params);

        surface_ = MaterialFactory.create(params.child_group("surface"), init_ctx);
        d_       = TextureFactory.create (params.child_group("d"),       init_ctx);
        A_       = TextureFactory.create (params.child_group("A"),       init_ctx);
        eta_     = TextureFactory.create (params.child_group("ior"), init_ctx);

        AGZ_HIERARCHY_WRAP("in initializing subsurface material")
    }

    ShadingPoint shade(const EntityIntersection &inct, Arena &arena) const override
    {
        Spectrum A   = A_  ->sample_spectrum(inct.uv);
        Spectrum d   = d_  ->sample_spectrum(inct.uv);
        real     eta = eta_->sample_real(inct.uv);

        auto shd = surface_->shade(inct, arena);
        shd.bssrdf = arena.create<ConstantBSSRDF>(
            inct, inct.geometry_coord, inct.user_coord, eta, A, d);

        return shd;
    }
};

AGZT_IMPLEMENTATION(Material, Subsurface, "subsurface")

AGZ_TRACER_END
