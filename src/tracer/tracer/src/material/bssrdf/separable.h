#pragma once

#include <agz/tracer/core/bssrdf.h>

AGZ_TRACER_BEGIN

class SeparableBSSRDF : public BSSRDF
{
    friend class SeparableBSDF;

    real sw(real cos_theta_i) const noexcept;

protected:

    Coord geometry_coord_;
    Coord shading_coord_;
    real eta_;

    virtual Spectrum distance_factor(real distance) const noexcept = 0;

    virtual real sample_distance(int channel, const Sample1 &sam) const noexcept = 0;

    virtual real pdf_distance(int channel, real dist) const noexcept = 0;

public:

    SeparableBSSRDF(const EntityIntersection &inct, const Coord &geometry_coord, const Coord &shading_coord, real eta) noexcept;

    BSSRDFSampleResult sample(TransportMode mode, const Sample4 &sam, Arena& arena) const override;

    real pdf(const SurfacePoint &xi, TransportMode mode) const noexcept override;
};

AGZ_TRACER_END
