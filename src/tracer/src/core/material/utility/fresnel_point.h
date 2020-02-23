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
    virtual Spectrum eval(real cos_theta_i) const noexcept = 0;
};

class ConductorPoint : public FresnelPoint
{
    Spectrum eta_out_;
    Spectrum eta_in_;
    Spectrum k_;

    Spectrum eta_2_, eta_k_2_;

public:

    ConductorPoint(const Spectrum &eta_out, const Spectrum &eta_in, const Spectrum &k) noexcept
    {
        eta_out_ = eta_out;
        eta_in_ = eta_in;
        k_ = k;

        eta_2_ = eta_in_ / eta_out_;
        eta_2_ *= eta_2_;

        eta_k_2_ = k_ / eta_out_;
        eta_k_2_ *= eta_k_2_;
    }

    Spectrum eval(real cos_theta_i) const noexcept override
    {
        if(cos_theta_i <= 0)
            return Spectrum();

        const real cos2 = cos_theta_i * cos_theta_i;
        const real sin2 = (std::max)(real(0), 1 - cos2);

        real(*p_sqrt)(real) = &std::sqrt;

        const Spectrum t0 = eta_2_ - eta_k_2_ - sin2;
        const Spectrum a2b2 = (t0 * t0 + real(4) * eta_2_ * eta_k_2_).map(p_sqrt);
        const Spectrum t1 = a2b2 + cos2;
        const Spectrum a = (real(0.5) * (a2b2 + t0)).map(p_sqrt);
        const Spectrum t2 = 2 * cos_theta_i * a;
        const Spectrum rs = (t1 - t2) / (t1 + t2);

        const Spectrum t3 = cos2 * a2b2 + sin2 * sin2;
        const Spectrum t4 = t2 * sin2;
        const Spectrum rp = rs * (t3 - t4) / (t3 + t4);

        return real(0.5) * (rp + rs);
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

    Spectrum eval(real cos_theta_i) const noexcept override
    {
        return Spectrum(refl_aux::dielectric_fresnel(eta_i_, eta_o_, cos_theta_i));
    }

    real eta_i() const noexcept { return eta_i_; }
    real eta_o() const noexcept { return eta_o_; }
};

AGZ_TRACER_END
