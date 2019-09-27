#pragma once

#include <agz/tracer/core/bssrdf.h>

#include "../../medium/phase_function.h"

AGZ_TRACER_BEGIN

class PathTracedBSSRDF : public BSSRDF
{
    Vec3 xo_wi_;
    Spectrum sigma_s_;
    Spectrum sigma_t_;

    HenyeyGreensteinPhaseFunction phase_function_;

public:

    PathTracedBSSRDF(
        const EntityIntersection &inct, const Vec3 &xo_wi,
        const Spectrum &sigma_a, const Spectrum &sigma_s, real g);

    BSSRDFSampleResult sample(TransportMode mode, const Sample4 &sam, Arena &arena) const override;
};

AGZ_TRACER_END
