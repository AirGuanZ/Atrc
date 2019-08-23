#pragma once

#include <agz/tracer/core/texture.h>

AGZ_TRACER_BEGIN

class NormalMapper : public misc::uncopyable_t
{
    const Texture *normal_map_ = nullptr;

public:

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx)
    {
        if(auto grp = params.find_child_group("normal_map"))
            normal_map_ = TextureFactory.create(*grp, init_ctx);
    }

    Coord reorient(const Vec2 &uv, const Coord &old_user_coord) const noexcept
    {
        if(!normal_map_)
            return old_user_coord;

        Spectrum local_nor_spec = normal_map_->sample_spectrum(uv);
        Vec3 local_nor = {
            local_nor_spec.r * 2 - 1,
            local_nor_spec.g * 2 - 1,
            local_nor_spec.b * 2 - 1
        };

        Vec3 world_nor = old_user_coord.local_to_global(local_nor.normalize());
        return old_user_coord.rotate_to_new_z(world_nor);
    }
};

AGZ_TRACER_END
