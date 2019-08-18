#pragma once

#include <agz/tracer_utility/math.h>

AGZ_TRACER_BEGIN

namespace microfacet
{

    inline real one_minus_5(real x) noexcept { real t = 1 - x, t2 = t * t; return t2 * t2 * t; }

    real gtr2(real cos_theta_h, real alpha) noexcept;

    real smith_gtr2(real tan_theta, real alpha) noexcept;

    Vec3 sample_gtr2(real alpha, const Sample2 &sample) noexcept;

    real anisotropic_gtr2(real sin_phi_h, real cos_phi_h, real sin_theta_h, real cos_theta_h, real ax, real ay) noexcept;

    real smith_anisotropic_gtr2(real cos_phi, real sin_phi, real ax, real ay, real tan_theta) noexcept;

    Vec3 sample_anisotropic_gtr2(real ax, real ay, const Vec2 &sample) noexcept;

    real gtr1(real sin_theta_h, real cos_theta_h, real alpha) noexcept;

    Vec3 sample_gtr1(real alpha, const Vec2 &sample) noexcept;

} // namespace microfacet

AGZ_TRACER_END
