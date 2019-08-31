#pragma once

#include <agz/tracer/core/intersection.h>
#include <agz/tracer_utility/object.h>

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

    virtual bool has_intersection(const Ray &r) const noexcept = 0;

    virtual bool closest_intersection(const Ray &r, EntityIntersection *inct) const noexcept = 0;

    virtual AABB world_bound() const noexcept = 0;
};

AGZT_INTERFACE(Aggregate)

AGZ_TRACER_END
