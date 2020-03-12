#pragma once

#include <optional>

#include <agz/tracer/core/intersection.h>
#include <agz/tracer/core/sampler.h>

AGZ_TRACER_BEGIN

/**
 * @brief result of sampling outer-scattering event
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
 * @brief participating medium interface
 */
class Medium
{
public:

    virtual ~Medium() = default;

    /**
     * @brief maximum continuous scattering event count in this medium
     *
     * e.g. 1 means only single scattering is considered
     */
    virtual int get_max_scattering_count() const noexcept = 0;

    /**
     * @brief transmittance between two points
     */
    virtual Spectrum tr(
        const Vec3 &a, const Vec3 &b, Sampler &sampler) const noexcept = 0;

    /**
     * @brief sample out-scattering event from a to b
     */
    virtual SampleOutScatteringResult sample_scattering(
        const Vec3 &a, const Vec3 &b, Sampler &sampler, Arena &arena) const = 0;
};

/**
 * @brief medium interface on entity surface point
 */
struct MediumInterface
{
    RC<const Medium> in;
    RC<const Medium> out;
};

AGZ_TRACER_END
