#pragma once

#include "./separable.h"

AGZ_TRACER_BEGIN

class NormalizedDiffusionBSSRDF : public SeparableBSSRDF
{
    Spectrum A_;
    Spectrum d_;

protected:

    Spectrum distance_factor(real distance) const noexcept override;

    real sample_distance(int channel, const Sample1 &sam) const noexcept override;

    real pdf_distance(int channel, real dist) const noexcept override;

public:

    NormalizedDiffusionBSSRDF(
        const EntityIntersection &inct, const Coord &geometry_coord, const Coord &shading_coord,
        real eta, real transparency,
        const Spectrum &A, const Spectrum &d) noexcept;
};

AGZ_TRACER_END
