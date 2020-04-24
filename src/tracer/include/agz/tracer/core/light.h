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
    LightSampleResult(
        const Vec3 &ref, const Vec3 &pos, const Vec3 &nor,
        const Spectrum &rad, real pdf) noexcept
        : ref(ref), pos(pos), nor(nor), radiance(rad), pdf(pdf)
    {
        
    }

    Vec3 ref;
    Vec3 pos;
    Vec3 nor;
    Spectrum radiance;
    real pdf;

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
inline const LightSampleResult LIGHT_SAMPLE_RESULT_NULL =
    { { }, { }, { }, { }, 0 };

/**
 * @brief result of sampling light emission
 */
struct LightEmitResult
{
    LightEmitResult(
        const Vec3 &pos, const Vec3 &dir, const Vec3 &nor, const Vec2 &uv,
        const Spectrum &rad, real pdf_pos, real pdf_dir) noexcept
        : pos(pos), dir(dir), nor(nor), uv(uv),
          radiance(rad), pdf_pos(pdf_pos), pdf_dir(pdf_dir)
    {
        
    }

    Vec3 pos;          // emitting position
    Vec3 dir;          // emitting direction
    Vec3 nor;          // normal at pos
    Vec2 uv;
    Spectrum radiance; // emitted radiance
    real pdf_pos;      // pdf w.r.t. light surface area
    real pdf_dir;      // pdf w.r.t. solid angle at position
};

/**
 * @brief pdf of sampling light emission
 */
struct LightEmitPDFResult
{
    LightEmitPDFResult (real pdf_pos, real pdf_dir) noexcept
        : pdf_pos(pdf_pos), pdf_dir(pdf_dir)
    {
        
    }

    real pdf_pos;
    real pdf_dir;
};

/**
 * @brief result of finding light emission position
 */
struct LightEmitPosResult
{
    LightEmitPosResult(const Vec3 &pos, const Vec3 &nor) noexcept
        : pos(pos), nor(nor)
    {
        
    }

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
    virtual LightSampleResult sample(
        const Vec3 &ref, const Sample5 &sam) const noexcept = 0;

    /**
     * @brief sample emission
     *
     * assert(ret.pdf_pos && ret.pdf_dir)
     */
    virtual LightEmitResult sample_emit(const Sample5 &sam) const noexcept = 0;

    /**
     * @brief pdf of sample_emit
     */
    virtual LightEmitPDFResult emit_pdf(
        const Vec3 &pos, const Vec3 &dir, const Vec3 &nor) const noexcept = 0;

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
    virtual Spectrum radiance(
        const Vec3 &pos, const Vec3 &nor, const Vec2 &uv,
        const Vec3 &light_to_out) const noexcept = 0;

    /**
     * @brief pdf of sample_wi (w.r.t. solid angle)
     *
     * @param ref reference point
     * @param pos position on light source
     * @param nor surface normal at position
     */
    virtual real pdf(
        const Vec3 &ref,
        const Vec3 &pos, const Vec3 &nor) const noexcept = 0;
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
    virtual Spectrum radiance(
        const Vec3 &ref, const Vec3 &ref_to_light) const noexcept = 0;

    /**
     * @brief pdf of sample_wi (w.r.t. solid angle)
     */
    virtual real pdf(
        const Vec3 &ref, const Vec3 &ref_to_light) const noexcept = 0;

    /**
     * @brief find the emitting pos according to ref point and emitting dir
     *
     * used in bidirectional rendering algorithms
     */
    virtual LightEmitPosResult emit_pos(
        const Vec3 &ref, const Vec3 &ref_to_light) const noexcept = 0;

    /**
     * @brief preprocess before the rendering start
     *
     * calling this method again will cover the previous result
     */
    void preprocess(const AABB &world_bound) noexcept;
};

AGZ_TRACER_END
