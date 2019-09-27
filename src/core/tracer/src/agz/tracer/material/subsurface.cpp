#include <agz/tracer/core/bssrdf.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture.h>

#include "./bssrdf/constant_bssrdf.h"

AGZ_TRACER_BEGIN

class Subsurface : public Material
{
    const Material *surface_ = nullptr;
    const Texture  *d_       = nullptr;
    const Texture  *A_       = nullptr;
    const Texture  *eta_     = nullptr;

public:

    explicit Subsurface(CustomedFlag customed_flag = DEFAULT_CUSTOMED_FLAG)
    {
        object_customed_flag_ = customed_flag;
    }

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

    void initialize(const Material *surface, const Texture *d, const Texture *A, const Texture *ior)
    {
        surface_ = surface;
        d_ = d;
        A_ = A;
        eta_ = ior;
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

Material *create_subsurface(
    const Material *surface,
    const Texture *d,
    const Texture *A,
    const Texture *ior,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena)
{
    auto ret = arena.create<Subsurface>(customed_flag);
    ret->initialize(surface, d, A, ior);
    return ret;
}

AGZT_IMPLEMENTATION(Material, Subsurface, "subsurface")

AGZ_TRACER_END
