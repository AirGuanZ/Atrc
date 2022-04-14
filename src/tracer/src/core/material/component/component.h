#pragma once

#include <agz/tracer/core/bsdf.h>

AGZ_TRACER_BEGIN

class BSDFComponent : public misc::uncopyable_t
{
public:

    struct SampleResult
    {
        FVec3 lwi;
        FSpectrum f;
        real pdf = 0;

        bool is_valid() const
        {
            return pdf > 0;
        }
    };

    struct BidirSampleResult
    {
        FVec3 lwi;
        FSpectrum f;
        real pdf = 0;
        real pdf_rev = 0;

        bool is_valid() const
        {
            return pdf > 0;
        }
    };

    virtual ~BSDFComponent() = default;

    virtual FSpectrum eval(const FVec3 &lwi, const FVec3 &lwo, TransMode mode) const = 0;

    virtual real pdf(const FVec3 &lwi, const FVec3 &lwo) const = 0;

    virtual SampleResult sample(const FVec3 &lwo, TransMode mode, const Sample2 &sam) const = 0;

    virtual BidirSampleResult sample_bidir(const FVec3 &lwo, TransMode mode, const Sample2 &sam) const = 0;

    virtual bool has_diffuse_component() const = 0;
};

inline BSDFComponent::SampleResult discard_pdf_rev(const BSDFComponent::BidirSampleResult &sample_result)
{
    BSDFComponent::SampleResult result;
    result.lwi = sample_result.lwi;
    result.f = sample_result.f;
    result.pdf = sample_result.pdf;
    return result;
}

AGZ_TRACER_END
