#pragma once

#include <agz/tracer/core/intersection.h>
#include <agz/tracer_utility/object.h>

AGZ_TRACER_BEGIN

/**
 * @brief 表面上某一点处的fresnel项
 */
class FresnelPoint
{
public:

    virtual ~FresnelPoint() = default;

    virtual Spectrum eval(real cos_theta_i) const noexcept = 0;

    // for dielectric only
    virtual real eta_i() const noexcept = 0;

    // for dielectric only
    virtual real eta_o() const noexcept = 0;
};

/**
 * @brief 材质中的fresnel项，用于接受uv以得到特定点的FresnelPoint
 */
class Fresnel : public obj::Object
{
public:

    using Object::Object;

    virtual FresnelPoint *get_point(const Vec2 &uv, Arena &arena) const = 0;
};

AGZT_INTERFACE(Fresnel)

AGZ_TRACER_END
