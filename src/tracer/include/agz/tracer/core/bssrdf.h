#pragma once

#include <agz/tracer/core/intersection.h>

AGZ_TRACER_BEGIN

struct BSSRDFSamplePiResult
{
    BSSRDFSamplePiResult(
        const EntityIntersection &inct,
        const FSpectrum &coef,
        real pdf) noexcept
        : inct(inct), coef(coef), pdf(pdf)
    {
        
    }

    EntityIntersection inct;
    FSpectrum          coef;
    real               pdf;
};

inline const BSSRDFSamplePiResult BSSRDF_SAMPLE_PI_RESULT_INVALID =
    BSSRDFSamplePiResult({}, {}, 0);

class BSSRDF : public misc::uncopyable_t
{
public:

    virtual ~BSSRDF() = default;

    virtual BSSRDFSamplePiResult sample_pi(
        const Sample3 &sam, Arena &arena) const = 0;
};

class BSSRDFSurface
{
public:

    virtual ~BSSRDFSurface() = default;

    virtual BSSRDF *create(const EntityIntersection &inct, Arena &arena) const
    {
        return nullptr;
    }
};

AGZ_TRACER_END  
