#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture.h>
#include "./bsdf_scaler.h"

AGZ_TRACER_BEGIN

class MatScaler : public Material
{
    const Material *internal_ = nullptr;
    const Texture *scale_     = nullptr;

public:

    explicit MatScaler(CustomedFlag customed_flag = DEFAULT_CUSTOMED_FLAG)
    {
        object_customed_flag_ = customed_flag;
    }

    static std::string description()
    {
        return R"___(
scale [Material]
    internal [Material] scaled material
    scale    [Texture]  scale map
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        init_customed_flag(params);

        internal_ = MaterialFactory.create(params.child_group("internal"), init_ctx);
        scale_ = TextureFactory.create(params.child_group("scale"), init_ctx);

        AGZ_HIERARCHY_WRAP("in initializing material scaler")
    }

    void initialize(const Material *internal, const Texture *scale)
    {
        internal_ = internal;
        scale_ = scale;
    }

    ShadingPoint shade(const EntityIntersection &inct, Arena &arena) const override
    {
        auto internal_shd = internal_->shade(inct, arena);
        auto scale = scale_->sample_spectrum(inct.uv);
        auto bsdf = arena.create<BSDFScaler>(scale, internal_shd.bsdf);
        return { bsdf };
    }
};

Material *create_mat_scaler(
    const Material *internal,
    const Texture *scale,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena)
{
    auto ret = arena.create<MatScaler>(customed_flag);
    ret->initialize(internal, scale);
    return ret;
}

AGZT_IMPLEMENTATION(Material, MatScaler, "scale")

AGZ_TRACER_END
