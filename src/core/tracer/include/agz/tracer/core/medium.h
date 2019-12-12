#pragma once

#include <optional>

#include <agz/tracer/core/intersection.h>
#include <agz/tracer/core/sampler.h>

AGZ_TRACER_BEGIN

/**
 * @brief 采样介质中发生的外散射的结果
 *
 * 可能会采样到一个散射点，也可能会采样到不发生散射。
 * 后一种情况下mediumInct为std::nullopt
 */
struct SampleOutScatteringResult
{
    MediumScattering scattering_point;
    Spectrum throughput;

    const BSDF *phase_function = nullptr;

    bool is_scattering_happened() const noexcept
    {
        return phase_function != nullptr;
    }
};

/**
 * @brief 介质接口
 */
class Medium
{
public:

    virtual ~Medium() = default;

    /**
     * @brief a与b点间的透射比
     */
    virtual Spectrum tr(const Vec3 &a, const Vec3 &b, Sampler &sampler) const noexcept = 0;

    /**
     * @brief 在从o向d的射线上采样一个散射点
     */
    virtual SampleOutScatteringResult sample_scattering(const Vec3 &a, const Vec3 &b, Sampler &sampler, Arena &arena) const noexcept = 0;
};

AGZ_TRACER_END
