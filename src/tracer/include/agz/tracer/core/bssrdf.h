#pragma once

#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/intersection.h>

AGZ_TRACER_BEGIN

struct BSSRDFSampleResult
{
    EntityIntersection inct;
    BSDF *bsdf = nullptr;

    Spectrum f;
    real pdf   = 0;

    bool valid() const noexcept
    {
        return bsdf != nullptr;
    }
};

inline const BSSRDFSampleResult BSSRDF_SAMPLE_RESULT_INVALID = { {}, nullptr, {}, 0 };

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

    virtual BSSRDFSampleResult sample(TransportMode mode, const Sample4 &sam, Arena &arena) const = 0;

    virtual real pdf(const SurfacePoint &xi, TransportMode mode) const noexcept = 0;
};

AGZ_TRACER_END
