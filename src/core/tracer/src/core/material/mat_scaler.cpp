#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture2d.h>
#include <agz/utility/misc.h>

#include "./bsdf_scaler.h"

AGZ_TRACER_BEGIN

class MatScaler : public Material
{
    std::shared_ptr<const Material> internal_;
    std::shared_ptr<const Texture2D> scale_;

public:

    void initialize(std::shared_ptr<const Material> internal, std::shared_ptr<const Texture2D> scale)
    {
        internal_ = internal;
        scale_ = scale;
    }

    ShadingPoint shade(const EntityIntersection &inct, Arena &arena) const override
    {
        auto internal_shd = internal_->shade(inct, arena);
        auto scale = scale_->sample_spectrum(inct.uv);
        auto bsdf = arena.create<BSDFScaler>(scale, internal_shd.bsdf);
        return { bsdf, inct.user_coord.z };
    }
};

std::shared_ptr<Material> create_mat_scaler(
    std::shared_ptr<const Material> internal,
    std::shared_ptr<const Texture2D> scale)
{
    auto ret = std::make_shared<MatScaler>();
    ret->initialize(internal, scale);
    return ret;
}

AGZ_TRACER_END
