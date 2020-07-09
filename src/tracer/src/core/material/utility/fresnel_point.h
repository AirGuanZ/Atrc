#pragma once

#include <agz/tracer/utility/reflection.h>

AGZ_TRACER_BEGIN

/**
 * @brief fresnel term
 */
class FresnelPoint
{
public:

    virtual ~FresnelPoint() = default;

    // Fr(cos_theta_i)
    virtual FSpectrum eval(real cos_theta_i) const noexcept = 0;
};

class ConductorPoint : public FresnelPoint
{
    FSpectrum eta_out_;
    FSpectrum eta_in_;
    FSpectrum k_;

    FSpectrum eta_2_, eta_k_2_;

public:

    ConductorPoint(
        const FSpectrum &eta_out, const FSpectrum &eta_in, const FSpectrum &k) noexcept
    {
        eta_out_ = eta_out;
        eta_in_ = eta_in;
        k_ = k;

        eta_2_ = eta_in_ / eta_out_;
        eta_2_ *= eta_2_;

        eta_k_2_ = k_ / eta_out_;
        eta_k_2_ *= eta_k_2_;
    }

    FSpectrum eval(real cos_theta_i) const noexcept override
    {
        if(cos_theta_i <= 0)
            return FSpectrum();

        const real cos2 = cos_theta_i * cos_theta_i;
        const real sin2 = (std::max)(real(0), 1 - cos2);

        const FSpectrum t0 = eta_2_ - eta_k_2_ - sin2;
        const FSpectrum a2b2 = sqrt((t0 * t0 + real(4) * eta_2_ * eta_k_2_));
        const FSpectrum t1 = a2b2 + cos2;
        const FSpectrum a = sqrt((real(0.5) * (a2b2 + t0)));
        const FSpectrum t2 = 2 * cos_theta_i * a;
        const FSpectrum rs = (t1 - t2) / (t1 + t2);

        const FSpectrum t3 = cos2 * a2b2 + sin2 * sin2;
        const FSpectrum t4 = t2 * sin2;
        const FSpectrum rp = rs * (t3 - t4) / (t3 + t4);

        return real(0.5) * (rp + rs);
    }
};

class ColoredConductorPoint : public ConductorPoint
{
    FSpectrum color_;

public:

    ColoredConductorPoint(
        const FSpectrum &color,
        const FSpectrum &eta_out, const FSpectrum &eta_in, const FSpectrum &k) noexcept
        : ConductorPoint(eta_out, eta_in, k)
    {
        color_ = color;
    }

    FSpectrum eval(real cos_theta_i) const noexcept override
    {
        return color_ * ConductorPoint::eval(cos_theta_i);
    }
};

class DielectricFresnelPoint : public FresnelPoint
{
    real eta_i_, eta_o_;

public:

    DielectricFresnelPoint(real eta_in, real eta_out) noexcept
        : eta_i_(eta_in), eta_o_(eta_out)
    {
        
    }

    FSpectrum eval(real cos_theta_i) const noexcept override
    {
        return FSpectrum(refl_aux::dielectric_fresnel(
                    eta_i_, eta_o_, cos_theta_i));
    }

    real eta_i() const noexcept { return eta_i_; }
    real eta_o() const noexcept { return eta_o_; }
};

AGZ_TRACER_END
