#pragma once

#include <agz/tracer/core/bssrdf.h>

AGZ_TRACER_BEGIN

class SeparableBSSRDF : public BSSRDF
{
public:

    SeparableBSSRDF(const EntityIntersection &po, real eta);

    BSSRDFSamplePiResult sample_pi(
        const Sample3 &sam, Arena &arena) const override;

protected:

    struct SampleRResult
    {
        FSpectrum coef;
        real distance = 0;
        real pdf      = 0;
    };

    virtual SampleRResult sample_r(int channel, Sample1 sam) const = 0;

    virtual FSpectrum eval_r(real distance) const = 0;

    virtual real pdf_r(int channel, real distance) const = 0;

    real eta_;

private:

    real pdf_pi(const EntityIntersection &pi) const;

    EntityIntersection po_;
};

AGZ_TRACER_END
