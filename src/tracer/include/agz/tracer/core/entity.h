#pragma once

#include <agz/tracer/core/intersection.h>

AGZ_TRACER_BEGIN

class AreaLight;

/**
 * @brief entity interface, representing visible object in scene
 *
 * all entities are in the world space
 */
class Entity
{
    bool no_denoise_ = false;

public:

    virtual ~Entity() = default;

    /** @brief set the cancel-denoising flag */
    void set_no_denoise_flag(bool flag) noexcept { no_denoise_ = flag; }

    /** @brief is cancel-denoising flag on? */
    bool get_no_denoise_flag() const noexcept { return no_denoise_; }

    /** @brief is there an intersection with given ray? */
    virtual bool has_intersection(const Ray &r) const noexcept = 0;

    /**
     * @brief find closest intersection with given ray
     * 
     * @param r ray
     * @param inct intersection. only be modified when returning true
     * 
     * @return whether there is an intersection
     */
    virtual bool closest_intersection(
        const Ray &r, EntityIntersection *inct) const noexcept = 0;

    /**
     * @brief aabb in world space
     */
    virtual AABB world_bound() const noexcept = 0;

    /**
     * @brief area light interface of this entity.
     *  nullptr means this is not a light source
     */
    virtual const AreaLight *as_light() const noexcept = 0;

    /**
     * @brief area light interface of this entity.
     *  nullptr means this is not a light source
     */
    virtual AreaLight *as_light() noexcept = 0;
};

AGZ_TRACER_END
