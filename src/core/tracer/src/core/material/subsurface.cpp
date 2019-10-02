#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture.h>
#include <agz/utility/misc.h>

#include "./bssrdf/constant_bssrdf.h"

AGZ_TRACER_BEGIN

class Subsurface : public Material
{
    std::shared_ptr<const Material> surface_;
    std::shared_ptr<const Texture> d_;
    std::shared_ptr<const Texture> A_;
    std::shared_ptr<const Texture> eta_;
    std::shared_ptr<const Texture> transparency_;

public:

    void initialize(
        std::shared_ptr<const Material> surface,
        std::shared_ptr<const Texture> d,
        std::shared_ptr<const Texture> A,
        std::shared_ptr<const Texture> ior,
        std::shared_ptr<const Texture> transparency)
    {
        surface_ = surface;
        d_ = d;
        A_ = A;
        eta_ = ior;
        transparency_ = transparency;
    }

    ShadingPoint shade(const EntityIntersection &inct, Arena &arena) const override
    {
        Spectrum A   = A_  ->sample_spectrum(inct.uv);
        Spectrum d   = d_  ->sample_spectrum(inct.uv);
        real     eta = eta_->sample_real(inct.uv);

        real transparency = transparency_->sample_real(inct.uv);

        auto shd = surface_->shade(inct, arena);
        shd.bssrdf = arena.create<ConstantBSSRDF>(
            inct, inct.geometry_coord, inct.user_coord, eta, transparency, A, d);

        return shd;
    }
};

std::shared_ptr<Material> create_subsurface(
    std::shared_ptr<const Material> surface,
    std::shared_ptr<const Texture> d,
    std::shared_ptr<const Texture> A,
    std::shared_ptr<const Texture> ior,
    std::shared_ptr<const Texture> transparency)
{
    auto ret = std::make_shared<Subsurface>();
    ret->initialize(surface, d, A, ior, transparency);
    return ret;
}

AGZ_TRACER_END
