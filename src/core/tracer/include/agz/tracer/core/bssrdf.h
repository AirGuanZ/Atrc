#pragma once

#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/intersection.h>

AGZ_TRACER_BEGIN

/**
 * @brief bssrdf采样结果
 */
struct BSSRDFSampleResult
{
    EntityIntersection inct; // 入射点
    BSDF *bsdf = nullptr;    // bsdf at wi; only transmission is considered

    Spectrum f;              // throughput
    real pdf   = 0;          // pdf w.r.t area

    bool valid() const noexcept
    {
        return bsdf != nullptr;
    }
};

/**
 * @brief 采样bssrdf失败时返回的结果
 */
inline const BSSRDFSampleResult BSSRDF_SAMPLE_RESULT_INVALID = { {}, nullptr, {}, 0 };

/**
 * @brief Bidirectional scattering-surface reflectance distribution function
 * 
 * see https://en.wikipedia.org/wiki/Bidirectional_scattering_distribution_function
 */
class BSSRDF : public misc::uncopyable_t
{
protected:

    EntityIntersection xo_;

public:

    explicit BSSRDF(const EntityIntersection &inct) noexcept
        : xo_(inct)
    {
        
    }

    virtual ~BSSRDF() = default;

    /**
     * @brief 采样入射点xi
     */
    virtual BSSRDFSampleResult sample(TransportMode mode, const Sample4 &sam, Arena &arena) const = 0;
};

AGZ_TRACER_END
