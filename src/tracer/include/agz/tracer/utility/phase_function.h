#pragma once

#include <agz/tracer/core/bsdf.h>

AGZ_TRACER_BEGIN

// See http://www.astro.umd.edu/~jph/HG_note.pdf
class HenyeyGreensteinPhaseFunction : public BSDF
{
    real g_ = 0;
    Spectrum albedo_;

    real phase_func(real u) const noexcept
    {
        const real g2 = g_ * g_;
        const real dem = 1 + g2 - 2 * g_ * u;
        return (1 - g2) / (4 * PI_r * dem * std::sqrt(dem));
    }

public:

    HenyeyGreensteinPhaseFunction(real g, const Spectrum &albedo) noexcept
        : g_(g), albedo_(albedo)
    {

    }

    Spectrum eval(
        const Vec3 &wi, const Vec3 &wo,
        TransMode mode, uint8_t) const noexcept override
    {
        const real u = -cos(wi, wo);
        return Spectrum(phase_func(u));
    }

    BSDFSampleResult sample(
        const Vec3 &wo, TransMode,
        const Sample3 &sam, uint8_t) const noexcept override
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

        const Vec3 local_wi = {
            sin_theta * std::sin(phi),
            sin_theta * std::cos(phi),
            cos_theta
        };

        const real phase_value = phase_func(u);

        return BSDFSampleResult(
            Coord::from_z(wo).local_to_global(local_wi),
            Spectrum(phase_value),
            phase_value,
            false);
    }

    real pdf(
        const Vec3 &wi, const Vec3 &wo, uint8_t) const noexcept override
    {
        return phase_func(-cos(wi, wo));
    }

    Spectrum albedo() const noexcept override
    {
        return albedo_;
    }

    bool is_delta() const noexcept override
    {
        return false;
    }

    bool has_diffuse_component() const noexcept override
    {
        return true;
    }
};

AGZ_TRACER_END
