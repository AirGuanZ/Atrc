#pragma once

#include "./separable.h"

AGZ_TRACER_BEGIN

// See http://graphics.pixar.com/library/ApproxBSSRDF/paper.pdf
class NormalizedDiffusionBSSRDF : public SeparableBSSRDF
{
    FSpectrum A_;
    FSpectrum s_;
    FSpectrum l_;
    FSpectrum d_;

protected:

    FSpectrum eval_r(real distance) const override;

    SampleRResult sample_r(int channel, Sample1 sam) const override;

    real pdf_r(int channel, real distance) const override;

public:

    NormalizedDiffusionBSSRDF(
        const EntityIntersection &inct, real eta,
        const FSpectrum &A, const FSpectrum &dmfp) noexcept;
};

AGZ_TRACER_END
