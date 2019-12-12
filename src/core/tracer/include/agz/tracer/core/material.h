#pragma once

#include <agz/tracer/core/intersection.h>
#include <agz/tracer/core/texture2d.h>

AGZ_TRACER_BEGIN

/**
 * @brief 不同类型介质分界面间的材质模型接口
 *
 * 负责 交点 -> ShadingPoint，仅此而已
 */
class Material
{
public:

    virtual ~Material() = default;

    /** @brief 将射线与实体的交点转换为shading point */
    virtual ShadingPoint shade(const EntityIntersection &inct, Arena &arena) const = 0;
};

/**
 * @brief 法线映射辅助设施
 */
class NormalMapper : public misc::uncopyable_t
{
    std::shared_ptr<const Texture2D> normal_map_;

public:

    NormalMapper() = default;

    explicit NormalMapper(std::shared_ptr<Texture2D> normal_map) noexcept
        : normal_map_(std::move(normal_map))
    {
        
    }

    /**
     * @brief 利用法线贴图中的信息偏转局部坐标系的坐标轴
     */
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
