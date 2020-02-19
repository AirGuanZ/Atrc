#pragma once

#include <agz/tracer/core/intersection.h>

AGZ_TRACER_BEGIN

/**
 * @brief fresnel term
 */
class FresnelPoint
{
public:

    virtual ~FresnelPoint() = default;

    // Fr(cos_theta_i)
    virtual Spectrum eval(real cos_theta_i) const noexcept = 0;

    // inner ior; for dielectric only
    virtual real eta_i() const noexcept = 0;

    // outer ior; for dielectric only
    virtual real eta_o() const noexcept = 0;
};

/**
 * @brief map uv to FresnelPoint
 */
class Fresnel
{
public:

    virtual ~Fresnel() = default;

    virtual FresnelPoint *get_point(const Vec2 &uv, Arena &arena) const = 0;
};

AGZ_TRACER_END
