#pragma once

#include <agz/tracer/core/intersection.h>
#include <agz/tracer/core/object.h>

AGZ_TRACER_BEGIN

class Entity;

/**
 * @brief 实体聚合接口，表示针对一组实体实现的求交加速数据结构
 */
class Aggregate : public obj::Object
{
public:

    using Object::Object;

    /**
     * @brief 建立静态加速数据结构
     * 
     * 添加完所有实体后、开始渲染前调用
     */
    virtual void build(const Entity *const*entities, size_t entity_count) = 0;

    /**
     * @brief 测试射线与实体有交点
     */
    virtual bool has_intersection(const Ray &r) const noexcept = 0;

    /**
     * @brief 寻找与射线起点最近的交点
     */
    virtual bool closest_intersection(const Ray &r, EntityIntersection *inct) const noexcept = 0;

    /**
     * @brief 所有实体在world space中的bounding box
     */
    virtual AABB world_bound() const noexcept = 0;
};

AGZT_INTERFACE(Aggregate)

AGZ_TRACER_END
