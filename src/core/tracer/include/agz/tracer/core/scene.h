#pragma once

#include <agz/utility/misc.h>

#include <agz/tracer/core/aggregate.h>

AGZ_TRACER_BEGIN

class Camera;
class Entity;
class Light;
class AreaLight;
class EnvirLight;

struct EntityIntersection;
class ScatteringPoint;

/**
 * @brief result of sampling light sources
 */
struct SceneSampleLightResult
{
    const Light *light;
    real pdf;
};

/**
 * @brief scene manager interface
 */
class Scene
{
public:

    virtual ~Scene() = default;

    /** @brief set current active camera */
    virtual void set_camera(std::shared_ptr<const Camera> camera) = 0;

    /** @brief get current active camera */
    virtual const Camera *get_camera() const noexcept = 0;

    /** @brief get all light sources */
    virtual misc::span<const Light* const> lights() const noexcept = 0;

    /** @brief get environment light */
    virtual const EnvirLight *envir_light() const noexcept = 0;

    /** @brief sample a light source */
    virtual SceneSampleLightResult sample_light(const Sample1 &sam) const noexcept = 0;

    /**
     * @brief pdf of sample_light
     *
     * assert(light is in scene)
     */
    virtual real light_pdf(const Light *light) const noexcept = 0;

    /** @brief is there an intersection with given ray */
    virtual bool has_intersection(const Ray &r) const noexcept = 0;

    /**
     * @brief is there no intersection between two given points
     */
    virtual bool visible(const Vec3 &A, const Vec3 &B) const noexcept = 0;

    /**
     * @brief find closest intersection with given ray
     *
     * note that *inct may be modified even there is no intersection at all
     *
     * @return whether there is an intersection
     */
    virtual bool closest_intersection(const Ray &r, EntityIntersection *inct) const noexcept = 0;

    /**
     * @brief must be called before rendering
     */
    virtual void start_rendering() = 0;
};

AGZ_TRACER_END
