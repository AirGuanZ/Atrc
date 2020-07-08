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
        const real root = alpha * tan_theta;
        return 2 / (1 + std::sqrt(1 + root * root));
    }

    FVec3 sample_gtr2(real alpha, const Sample2 &sample) noexcept
    {
        const real phi = 2 * PI_r * sample.u;
        const real cos_theta = std::sqrt((1 - sample.v)
                             / (1 + (sqr(alpha) - 1) * sample.v));
        const real sin_theta = local_angle::cos_2_sin(cos_theta);

        return FVec3(
            sin_theta * std::cos(phi),
            sin_theta * std::sin(phi),
            cos_theta).normalize();
    }

    real anisotropic_gtr2(
        real sin_phi_h, real cos_phi_h,
        real sin_theta_h, real cos_theta_h,
        real ax, real ay) noexcept
    {
        const real A = compute_a(sin_phi_h, cos_phi_h, ax, ay);
        const real RD = sqr(sin_theta_h) * A + sqr(cos_theta_h);
        return 1 / (PI_r * ax * ay * sqr(RD));
    }

    real smith_anisotropic_gtr2(
        real cos_phi, real sin_phi, real ax, real ay, real tan_theta) noexcept
    {
        const real t = sqr(ax * cos_phi) + sqr(ay * sin_phi);
        const real sqr_val = 1 + t * sqr(tan_theta);
        const real lambda = -real(0.5) + real(0.5) * std::sqrt(sqr_val);
        return 1 / (1 + lambda);
    }

    FVec3 sample_anisotropic_gtr2(
        real ax, real ay, const Sample2 &sample) noexcept
    {
        real sin_phi_h = ay * std::sin(2 * PI_r * sample.u);
        real cos_phi_h = ax * std::cos(2 * PI_r * sample.u);
        const real nor = 1 / std::sqrt(sqr(sin_phi_h) + sqr(cos_phi_h));
        sin_phi_h *= nor;
        cos_phi_h *= nor;

        const real A = compute_a(sin_phi_h, cos_phi_h, ax, ay);
        const real cos_theta_h = std::sqrt(A * (1 - sample.v)
                              / ((1 - A) * sample.v + A));
        const real sin_theta_h = std::sqrt(
            (std::max<real>)(0, 1 - sqr(cos_theta_h)));

        return FVec3(
            sin_theta_h * cos_phi_h,
            sin_theta_h * sin_phi_h,
            cos_theta_h).normalize();
    }

    real gtr1(real sin_theta_h, real cos_theta_h, real alpha) noexcept
    {
        const real U = sqr(alpha) - 1;
        const real LD = 2 * PI_r  * std::log(alpha);
        const real RD = sqr(alpha * cos_theta_h) + sqr(sin_theta_h);
        return U / (LD * RD);
    }

    FVec3 sample_gtr1(real alpha, const Sample2 &sample) noexcept
    {
        const real phi = 2 * PI_r * sample.u;
        const real cos_theta = std::sqrt((std::pow(alpha, 2 - 2 * sample.v) - 1)
                             / (sqr(alpha) - 1));
        const real sin_theta = local_angle::cos_2_sin(cos_theta);
        return FVec3(
            sin_theta * std::cos(phi),
            sin_theta * std::sin(phi),
            cos_theta).normalize();
    }

    FVec3 sample_anisotropic_gtr2_vnor(
        const FVec3 &ve, real ax, real ay, const Sample2 &sam) noexcept
    {
        const FVec3 vh = FVec3(ax * ve.x, ay * ve.y, ve.z).normalize();
        const real lensq = vh.x * vh.x + vh.y * vh.y;

        const FVec3 t1 = lensq > EPS() ?
            FVec3(-vh.y, vh.x, 0) / std::sqrt(lensq) : FVec3(1, 0, 0);
        const FVec3 t2 = cross(vh, t1);

        const real r = std::sqrt(sam.u);
        const real phi = 2 * PI_r * sam.v;
        const real t_1 = r * std::cos(phi);
        const real _t_2 = r * std::sin(phi);
        const real s = real(0.5) * (1 + vh.z);
        const real t_2 = (1 - s) * std::sqrt(1 - t_1 * t_1) + s * _t_2;

        const FVec3 nh = t_1 * t1 + t_2 * t2 +
                        std::sqrt((std::max)(real(0),
                                             1 - t_1 * t_1 - t_2 * t_2)) * vh;
        const FVec3 ne = FVec3(
            ax * nh.x, ay * nh.y, (std::max)(real(0), nh.z)).normalize();

        return ne;
    }

} // namespace microfacet

AGZ_TRACER_END
