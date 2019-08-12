#pragma once

#include <agz/tracer/core/intersection.h>
#include <agz/tracer/utility/object.h>

AGZ_TRACER_BEGIN

/**
 * @brief 沿射线采样介质的结果
 * 
 * invalid()为true表示采样失败，没有下一个scattering point
 * inct.invalid()为true表示没采样到介质
 * 其他情况表示采样到了介质
 */
struct SampleMediumResult
{
    MediumIntersection inct;
    real pdf;

    bool invalid() const noexcept
    {
        return !pdf;
    }
};

inline const SampleMediumResult SAMPLE_MEDIUM_RESULT_INVALID = { { }, 0 };

class Medium : public obj::Object
{
public:

    virtual Spectrum tr(const Vec3 &a, const Vec3 &b) const noexcept = 0;

    virtual SampleMediumResult sample(const Vec3 &o, const Vec3 &d, real t_min, real t_max, const Sample1 &sam) const noexcept = 0;

    virtual ShadingPoint shade(const MediumIntersection &inct, Arena &arena) const noexcept = 0;
};

AGZT_INTERFACE(Medium)

AGZ_TRACER_END
