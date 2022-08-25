﻿#include <agz/tracer/core/medium.h>
#include <agz/tracer/utility/phase_function.h>
#include <agz-utils/misc.h>

AGZ_TRACER_BEGIN

class HomogeneousMedium : public Medium
{
    FSpectrum sigma_a_;
    FSpectrum sigma_s_;
    FSpectrum sigma_t_;
    real g_ = 0;

    int max_scattering_count_;

    FSpectrum albedo() const
    {
        return !sigma_t_ ? FSpectrum(1) : sigma_s_ / sigma_t_;
    }

public:

    HomogeneousMedium(
        const FSpectrum &sigma_a, const FSpectrum &sigma_s, real g,
        int max_scattering_count)
    {
        AGZ_HIERARCHY_TRY

        sigma_a_ = sigma_a;
        sigma_s_ = sigma_s;
        sigma_t_ = sigma_a + sigma_s;

        g_ = g;
        if(g_ <= -1 || g_ >= 1)
            throw ObjectConstructionException(
                "invalid g value: " + std::to_string(g_));

        max_scattering_count_ = max_scattering_count;

        AGZ_HIERARCHY_WRAP("in initializing homogeneous medium")
    }

    int get_max_scattering_count() const noexcept override
    {
        return max_scattering_count_;
    }

    FSpectrum tr(
        const FVec3 &a, const FVec3 &b, Sampler &sampler) const noexcept override
    {
        const FSpectrum exp = -sigma_t_ * (a - b).length();
        return {
            std::exp(exp.r),
            std::exp(exp.g),
            std::exp(exp.b)
        };
    }

    FSpectrum ab(
        const FVec3 &a, const FVec3 &b, Sampler &sampler) const noexcept override
    {
        const FSpectrum exp = -sigma_a_ * (a - b).length();
        return {
            std::exp(exp.r),
            std::exp(exp.g),
            std::exp(exp.b)
        };
    }

    SampleOutScatteringResult sample_scattering(
        const FVec3 &a, const FVec3 &b,
        Sampler &sampler, Arena &arena, bool) const override
    {
        const Sample1 sam = sampler.sample1();
        if(!sigma_s_)
            return { { }, tr(a, b, sampler), nullptr };

        const real t_max = (b - a).length();
        const auto [color_channel, new_sam] = math::distribution
            ::extract_uniform_int(sam.u, 0, SPECTRUM_COMPONENT_COUNT);
        const real st = -std::log(new_sam) / sigma_t_[color_channel];

        const bool sample_medium = st < t_max;
        FSpectrum tr;
        for(int i = 0; i < SPECTRUM_COMPONENT_COUNT; ++i)
            tr[i] = std::exp(-sigma_t_[i] * (std::min)(st, t_max));
        const FSpectrum density = sample_medium ? sigma_t_ * tr : tr;

        real pdf = 0;
        for(int i = 0; i < SPECTRUM_COMPONENT_COUNT; ++i)
            pdf += density[i];
        pdf /= SPECTRUM_COMPONENT_COUNT;
        pdf = (std::max)(pdf, EPS());

        FSpectrum throughput = tr / pdf;
        if(sample_medium)
            throughput *= sigma_s_;

        if(sample_medium)
        {
            MediumScattering scattering_point;
            scattering_point.pos = lerp(a, b, st / t_max);
            scattering_point.medium = this;
            scattering_point.wr = (a - b) / t_max;

            auto phase_function = arena.create<HenyeyGreensteinPhaseFunction>(
                g_, albedo());

            return SampleOutScatteringResult(
                scattering_point, throughput, phase_function);
        }

        return SampleOutScatteringResult({}, throughput, nullptr);
    }
};

RC<Medium> create_homogeneous_medium(
    const FSpectrum &sigma_a,
    const FSpectrum &sigma_s,
    real g,
    int max_scattering_count)
{
    return newRC<HomogeneousMedium>(sigma_a, sigma_s, g, max_scattering_count);
}

AGZ_TRACER_END
