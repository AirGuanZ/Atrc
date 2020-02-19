#pragma once

#include <agz/tracer/core/intersection.h>

AGZ_TRACER_BEGIN

class Scene;
class AreaLight;
class EnvirLight;

/**
 * @brief result of sampling light wi
 */
struct LightSampleResult
{
    Vec3 ref;
    Vec3 pos;
    Vec3 nor;
    Spectrum radiance;
    real pdf = 0;

    bool valid() const noexcept
    {
        return pdf != 0;
    }

    Vec3 ref_to_light() const noexcept
    {
        return (pos - ref).normalize();
    }
};

/**
 * @brief return value when sampling light wi fails
 */
inline const LightSampleResult LIGHT_SAMPLE_RESULT_NULL = { { }, { }, { }, { }, 0 };

/**
 * @brief result of sampling light emission
 */
struct LightEmitResult
{
    Vec3 pos;          // emitting position
    Vec3 dir;          // emitting direction
    Vec3 nor;          // normal at pos
    Spectrum radiance; // emitted radiance
    real pdf_pos = 0;  // pdf w.r.t. light surface area
    real pdf_dir = 0;  // pdf w.r.t. solid angle at position
};

/**
 * @brief pdf of sampling light emission
 */
struct LightEmitPDFResult
{
    real pdf_pos = 0;
    real pdf_dir = 0;
};

/**
 * @brief result of finding light emission position
 */
struct LightEmitPosResult
{
    Vec3 pos;
    Vec3 nor;
};

/**
 * @brief light source interface
 *
 * light sources are divided into two categories: area light & environment light
 */
class Light
{
public:

    virtual ~Light() = default;

    /**
     * @brief is this an area light
     */
    virtual bool is_area() const noexcept = 0;

    /**
     * @brief get the area light source interface
     */
    virtual const AreaLight *as_area() const noexcept { return nullptr; }

    /**
     * @brief get the environment light source interface
     */
    virtual const EnvirLight *as_envir() const noexcept { return nullptr; }

    /**
     * @brief sample light wi at ref
     */
    virtual LightSampleResult sample(const Vec3 &ref, const Sample5 &sam) const noexcept = 0;

    /**
     * @brief sample emission
     *
     * assert(ret.pdf_pos && ret.pdf_dir)
     */
    virtual LightEmitResult sample_emit(const Sample5 &sam) const noexcept = 0;

    /**
     * @brief pdf of sample_emit
     */
    virtual LightEmitPDFResult emit_pdf(const Vec3 &position, const Vec3 &direction, const Vec3 &normal) const noexcept = 0;

    /**
     * @brief emission power
     */
    virtual Spectrum power() const noexcept = 0;
};

/**
 * @brief area light source interface
 */
class AreaLight : public Light
{
public:

    bool is_area() const noexcept override final { return true; }

    const AreaLight *as_area() const noexcept override final { return this; }

    /**
     * @brief eval light source radiance
     * 
     * @param pos position on light source
     * @param nor normal at pos
     * @param light_to_out direction from light source to outside
     */
    virtual Spectrum radiance(const Vec3 &pos, const Vec3 &nor, const Vec3 &light_to_out) const noexcept = 0;

    /**
     * @brief pdf of sample_wi (w.r.t. solid angle)
     *
     * @param ref reference point
     * @param spt point on light source
     */
    virtual real pdf(const Vec3 &ref, const SurfacePoint &spt) const noexcept = 0;
};

/**
 * @brief environment light source interface
 */
class EnvirLight : public Light
{
protected:

    real world_radius_ = 1;
    Vec3 world_centre_;

public:

    bool is_area() const noexcept override final { return false; }

    const EnvirLight *as_envir() const noexcept override final { return this; }

    /**
     * @brief eval light source radiance
     *
     * @param ref outside point
     * @param ref_to_light direction from outside to light source
     * @return radiance from light source to outside
     */
    virtual Spectrum radiance(const Vec3 &ref, const Vec3 &ref_to_light) const noexcept = 0;

    /**
     * @brief pdf of sample_wi (w.r.t. solid angle)
     */
    virtual real pdf(const Vec3 &ref, const Vec3 &ref_to_light) const noexcept = 0;

    /**
     * @brief find the emitting position according to outside point and emitting direction
     *
     * used in bidirectional rendering algorithms
     */
    virtual LightEmitPosResult emit_pos(const Vec3 &ref, const Vec3 &ref_to_light) const noexcept = 0;

    /**
     * @brief preprocess before the rendering start
     *
     * calling this method again will cover the previous result
     */
    void preprocess(const AABB &world_bound) noexcept;
};

AGZ_TRACER_END
