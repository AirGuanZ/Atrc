#pragma once

#include <agz/tracer/core/bsdf.h>

AGZ_TRACER_BEGIN

// See http://www.astro.umd.edu/~jph/HG_note.pdf
class HenyeyGreensteinPhaseFunction : public BSDF
{
    real g_ = 0;
    FSpectrum albedo_;

    real phase_func(real u) const
    {
        const real g2 = g_ * g_;
        const real dem = 1 + g2 - 2 * g_ * u;
        return (1 - g2) / (4 * PI_r * dem * std::sqrt(dem));
    }

public:

    HenyeyGreensteinPhaseFunction(real g, const FSpectrum &albedo)
        : g_(g), albedo_(albedo)
    {

    }

    FSpectrum eval(const FVec3 &wi, const FVec3 &wo, TransMode mode) const override
    {
        const real u = -cos(wi, wo);
        return FSpectrum(phase_func(u));
    }

    BSDFSampleResult sample(const FVec3 &wo, TransMode, const Sample3 &sam) const override
    {
        const real s = 2 * sam.u - 1;
        real u;
        if(std::abs(g_) < real(0.001))
            u = s;
        else
        {
            real g2 = g_ * g_;
            u = (1 + g2 - math::sqr((1 - g2) / (1 + g_ * s))) / (2 * g_);
        }

        const real cos_theta = -u;
        const real sin_theta = local_angle::cos_2_sin(cos_theta);
        const real phi = 2 * PI_r * sam.v;

        const FVec3 local_wi = {
            sin_theta * std::sin(phi),
            sin_theta * std::cos(phi),
            cos_theta
        };

        const real phase_value = phase_func(u);

        return BSDFSampleResult(
            FCoord::from_z(wo).local_to_global(local_wi),
            FSpectrum(phase_value),
            phase_value,
            false);
    }

    BSDFBidirSampleResult sample_bidir(const FVec3 &wo, TransMode mode, const Sample3 &sam) const override
    {
        auto t = sample(wo, mode, sam);
        return BSDFBidirSampleResult(t.dir, t.f, t.pdf, pdf(wo, t.dir), t.is_delta);
    }

    real pdf(const FVec3 &wi, const FVec3 &wo) const override
    {
        return phase_func(-cos(wi, wo));
    }

    FSpectrum albedo() const override
    {
        return albedo_;
    }

    bool is_delta() const override
    {
        return false;
    }

    bool has_diffuse_component() const override
    {
        return true;
    }
};

AGZ_TRACER_END
