#pragma once

#include <any>

#include <agz/tracer/core/intersection.h>

AGZ_TRACER_BEGIN

/**
 * @brief geometry object
 */
class Geometry
{
public:

    virtual ~Geometry() = default;

    /**
     * @brief is there an intersection with given ray
     */
    virtual bool has_intersection(const Ray &r) const noexcept = 0;

    /**
     * @brief find closest intersection with given ray
     *
     * @param r ray
     * @param inct intersection information. only be modified when true is returned
     *
     * @return whether there is an intersection
     */
    virtual bool closest_intersection(const Ray &r, GeometryIntersection *inct) const noexcept = 0;

    /**
     * @brief aabb in world space
     */
    virtual AABB world_bound() const noexcept = 0;

    /**
     * @brief surface area
     *
     * note that double-sided geometry object has 2x surface area value
     */
    virtual real surface_area() const noexcept = 0;

    /**
     * @brief sample a point on the geometry object
     * 
     * @param pdf w.r.t. surface area
     */
    virtual SurfacePoint sample(real *pdf, const Sample3 &sam) const noexcept = 0;

    /**
     * @brief pdf of sample
     */
    virtual real pdf(const Vec3 &pos) const noexcept = 0;

    /**
     * @brief sample a point on the geometry object
     * 
     * @param ref reference point. returned point must be visible from ref
     * @param pdf pdf w.r.t. surface area
     */
    virtual SurfacePoint sample(const Vec3 &ref, real *pdf, const Sample3 &sam) const noexcept = 0;

    /**
     *
     * @brief pdf of sample with ref
     */
    virtual real pdf(const Vec3 &ref, const Vec3 &pos) const noexcept = 0;
};

AGZ_TRACER_END
