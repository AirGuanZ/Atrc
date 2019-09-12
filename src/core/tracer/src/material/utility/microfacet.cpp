#include "./microfacet.h"

AGZ_TRACER_BEGIN

namespace microfacet
{
    
    real sqr(real x) noexcept
    {
        return x * x;
    }

    real compute_a(real sin_phi_h, real cos_phi_h, real ax, real ay) noexcept
    {
        return sqr(cos_phi_h / ax) + sqr(sin_phi_h / ay);
    }

    real gtr2(real cos_theta_h, real alpha) noexcept
    {
        return sqr(alpha) / (PI_r * sqr(1 + (sqr(alpha) - 1) * sqr(cos_theta_h)));
    }

    real smith_gtr2(real tan_theta, real alpha) noexcept
    {
        if(!tan_theta)
            return 1;
        real root = alpha * tan_theta;
        return 2 / (1 + std::sqrt(1 + root * root));
    }

    Vec3 sample_gtr2(real alpha, const Sample2 &sample) noexcept
    {
        real phi = 2 * PI_r * sample.u;
        real cos_theta = std::sqrt((1 - sample.v) / (1 + (sqr(alpha) - 1) * sample.v));
        real sin_theta = local_angle::cos_2_sin(cos_theta);
        return Vec3(sin_theta * std::cos(phi), sin_theta * std::sin(phi), cos_theta).normalize();
    }

    real anisotropic_gtr2(real sin_phi_h, real cos_phi_h, real sin_theta_h, real cos_theta_h, real ax, real ay) noexcept
    {
        real A = compute_a(sin_phi_h, cos_phi_h, ax, ay);
        real RD = sqr(sin_theta_h) * A + sqr(cos_theta_h);
        return 1 / (PI_r * ax * ay * sqr(RD));
    }

    real smith_anisotropic_gtr2(real cos_phi, real sin_phi, real ax, real ay, real tan_theta) noexcept
    {
        real t = sqr(ax * cos_phi) + sqr(ay * sin_phi);
        real sqr_val = 1 + t * sqr(tan_theta);
        real lambda = -real(0.5) + real(0.5) * std::sqrt(sqr_val);
        return 1 / (1 + lambda);
    }

    Vec3 sample_anisotropic_gtr2(real ax, real ay, const Sample2 &sample) noexcept
    {
        real sin_phi_h = ay * std::sin(2 * PI_r * sample.u);
        real cos_phi_h = ax * std::cos(2 * PI_r * sample.u);

        real nor = 1 / std::sqrt(sqr(sin_phi_h) + sqr(cos_phi_h));
        sin_phi_h *= nor;
        cos_phi_h *= nor;

        real A = compute_a(sin_phi_h, cos_phi_h, ax, ay);
        real cos_theta_h = std::sqrt(A * (1 - sample.v) / ((1 - A) * sample.v + A));
        real sin_theta_h = std::sqrt((std::max<real>)(0, 1 - sqr(cos_theta_h)));

        return Vec3(sin_theta_h * cos_phi_h, sin_theta_h * sin_phi_h, cos_theta_h).normalize();
    }

    real gtr1(real sin_theta_h, real cos_theta_h, real alpha) noexcept
    {
        real U = sqr(alpha) - 1;
        real LD = 2 * PI_r  * std::log(alpha);
        real RD = sqr(alpha * cos_theta_h) + sqr(sin_theta_h);
        return U / (LD * RD);
    }

    Vec3 sample_gtr1(real alpha, const Sample2 &sample) noexcept
    {
        real phi = 2 * PI_r * sample.u;
        real cos_theta = std::sqrt((std::pow(alpha, 2 - 2 * sample.v) - 1) / (sqr(alpha) - 1));
        real sin_theta = local_angle::cos_2_sin(cos_theta);
        return Vec3(sin_theta * std::cos(phi), sin_theta * std::sin(phi), cos_theta).normalize();
    }

} // namespace microfacet

AGZ_TRACER_END
