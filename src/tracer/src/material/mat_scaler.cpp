#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture.h>
#include "./bsdf_scaler.h"

AGZ_TRACER_BEGIN

class MatScaler : public Material
{
    const Material *internal_ = nullptr;
    const Texture *scale_     = nullptr;

public:

    using Material::Material;

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

        internal_ = MaterialFactory.create(params.child_group("internal"), init_ctx);
        scale_ = TextureFactory.create(params.child_group("scale"), init_ctx);

        AGZ_HIERARCHY_WRAP("in initializing material scaler")
    }

    ShadingPoint shade(const EntityIntersection &inct, Arena &arena) const override
    {
        auto internal_shd = internal_->shade(inct, arena);
        auto scale = scale_->sample_spectrum(inct.uv);
        auto bsdf = arena.create<BSDFScaler>(scale, internal_shd.bsdf);
        return { bsdf };
    }
};

AGZT_IMPLEMENTATION(Material, MatScaler, "scale")

AGZ_TRACER_END
