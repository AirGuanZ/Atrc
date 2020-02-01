#pragma once

#include <memory>
#include <vector>

#include <agz/tracer/core/intersection.h>

AGZ_TRACER_BEGIN

class Entity;

/**
 * @brief acceleration data structure interface between entities
 */
class Aggregate
{
public:

    virtual ~Aggregate() = default;

    /**
     * @brief build the data structure
     * 
     * must be called before rendering
     *
     * recall this method will rebuild the dt
     */
    virtual void build(const std::vector<std::shared_ptr<const Entity>> &entities) = 0;

    /**
     * @brief test whether an intersection exists
     */
    virtual bool has_intersection(const Ray &r) const noexcept = 0;

    /**
     * @brief find the closest intersection between ray and entities
     */
    virtual bool closest_intersection(const Ray &r, EntityIntersection *inct) const noexcept = 0;
};

AGZ_TRACER_END
