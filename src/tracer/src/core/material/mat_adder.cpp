#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture2d.h>
#include <agz/utility/misc.h>

#include "./bsdf_adder.h"

AGZ_TRACER_BEGIN

class MatAdder : public Material
{
    std::vector<std::shared_ptr<const Material>> mats_;

public:

    explicit MatAdder(std::vector<std::shared_ptr<const Material>> &&mats)
    {
        mats_ = std::move(mats);
    }

    ShadingPoint shade(const EntityIntersection &inct, Arena &arena) const override
    {
        auto bsdf = arena.create<BSDFAdder>();
        for(auto mat : mats_)
        {
            auto subbsdf = mat->shade(inct, arena);
            bsdf->add(subbsdf.bsdf);
        }
        return { bsdf, inct.user_coord.z };
    }
};

std::shared_ptr<Material> create_mat_adder(
    std::vector<std::shared_ptr<const Material>> &&mats)
{
    return std::make_shared<MatAdder>(std::move(mats));
}

AGZ_TRACER_END
