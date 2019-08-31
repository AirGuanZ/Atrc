#pragma once

#include <agz/tracer/core/intersection.h>
#include <agz/tracer_utility/object.h>

AGZ_TRACER_BEGIN

/**
 * @brief 不同类型介质分界面间的材质模型接口
 *
 * 负责 交点 -> ShadingPoint，仅此而已
 */
class Material : public obj::Object
{
public:

    using Object::Object;

    /** @brief 将射线与实体的交点转换为shading point */
    virtual ShadingPoint shade(const EntityIntersection &inct, Arena &arena) const = 0;
};

AGZT_INTERFACE(Material)

AGZ_TRACER_END
