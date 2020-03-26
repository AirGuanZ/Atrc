#pragma once

#include "./separable.h"

AGZ_TRACER_BEGIN

// See http://graphics.pixar.com/library/ApproxBSSRDF/paper.pdf
class NormalizedDiffusionBSSRDF : public SeparableBSSRDF
{
    Spectrum A_;
    Spectrum s_;
    Spectrum l_;
    Spectrum d_;

protected:

    Spectrum eval_r(real distance) const override;

    SampleRResult sample_r(int channel, Sample1 sam) const override;

    real pdf_r(int channel, real distance) const override;

public:

    NormalizedDiffusionBSSRDF(
        const EntityIntersection &inct, real eta,
        const Spectrum &A, const Spectrum &dmfp) noexcept;
};

AGZ_TRACER_END
