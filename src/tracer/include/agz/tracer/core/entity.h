#pragma once

#include <agz/tracer/core/intersection.h>
#include <agz/tracer/utility/object.h>

AGZ_TRACER_BEGIN

class Light;

/**
 * @brief 实体接口，表示场景中的可渲染物体
 * 
 * 所有的实体都被定义在世界坐标系中，原则上不允许实体间出现任何直接或间接的嵌套结构
 */
class Entity : public obj::Object
{
public:

    using Object::Object;

    /** @brief 判断给定射线是否与该实体有交点 */
    virtual bool has_intersection(const Ray &r) const noexcept = 0;

    /**
     * @brief 找到给定射线与该实体的最近交点
     * 
     * @param r 射线
     * @param inct 交点信息，其每个成员有效当且仅当返回值为true；若返回false，则inct必然未被修改
     * 
     * @return 存在交点时返回true，否则返回false
     */
    virtual bool closest_intersection(const Ray &r, EntityIntersection *inct) const noexcept = 0;

    /**
     * @brief 返回该实体在世界坐标系中的轴对齐包围盒
     */
    virtual AABB world_bound() const noexcept = 0;

    /**
     * @brief 返回自身作为area_light的接口。若并非光源，则返回nullptr。
     */
    virtual const Light *as_light() const noexcept = 0;

    /**
     * @brief 返回自身作为area_light的接口。若并非光源，则返回nullptr。
     */
    virtual Light *as_light() noexcept = 0;
};

AGZT_INTERFACE(Entity)

AGZ_TRACER_END
