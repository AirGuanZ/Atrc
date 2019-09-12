#pragma once

#include <agz/tracer/core/intersection.h>
#include <agz/tracer/core/object.h>

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
    real pdf = 0;            // w.r.t. dist. 为0表示采样失败

    bool invalid() const noexcept
    {
        return !pdf;
    }

    bool is_med() const noexcept
    {
        return !inct.invalid();
    }
};

/**
 * @brief 采样medium失败时的返回值
 */
inline const SampleMediumResult SAMPLE_MEDIUM_RESULT_INVALID = { { }, 0 };

/**
 * @brief 介质接口
 */
class Medium : public obj::Object
{
public:

    /**
     * @brief a与b点间的透射比
     */
    virtual Spectrum tr(const Vec3 &a, const Vec3 &b) const noexcept = 0;

    /**
     * @brief 在从o向d的射线上采样一个散射点
     */
    virtual SampleMediumResult sample(const Vec3 &o, const Vec3 &d, real t_min, real t_max, const Sample1 &sam) const noexcept = 0;

    /**
     * @brief 取得散射点的着色信息（phase function等）
     */
    virtual ShadingPoint shade(const MediumIntersection &inct, Arena &arena) const noexcept = 0;
};

AGZT_INTERFACE(Medium)

AGZ_TRACER_END
