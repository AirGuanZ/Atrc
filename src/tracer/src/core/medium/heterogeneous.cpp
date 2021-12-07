#include <agz/tracer/core/medium.h>
#include <agz/tracer/core/texture3d.h>
#include <agz/tracer/utility/phase_function.h>
#include <agz-utils/misc.h>
#include <agz-utils/texture.h>

AGZ_TRACER_BEGIN

class HeterogeneousMedium : public Medium
{
    FTransform3 local_to_world_;

    RC<const Texture3D> density_;
    RC<const Texture3D> albedo_;
    RC<const Texture3D> g_;

    real max_density_;
    real inv_max_density_;

    int max_scattering_count_;
    bool white_for_indiect_;

public:

    HeterogeneousMedium(
        const FTransform3 &local_to_world,
        RC<const Texture3D> density,
        RC<const Texture3D> albedo,
        RC<const Texture3D> g,
        int max_scattering_count,
        bool white_for_indirect)
    {
        local_to_world_ = local_to_world;

        density_ = std::move(density);
        albedo_  = std::move(albedo);
        g_       = std::move(g);

        max_density_ = density_->max_real();
        max_density_ = (std::max)(max_density_, EPS());
        inv_max_density_ = 1 / max_density_;

        max_scattering_count_ = max_scattering_count;
        white_for_indiect_ = white_for_indirect;
    }

    int get_max_scattering_count() const noexcept override
    {
        return max_scattering_count_;
    }

    FSpectrum tr(
        const FVec3 &a, const FVec3 &b, Sampler &sampler) const noexcept override
    {
        real result = 1;
        real t = 0;
        const real t_max = distance(a, b);

        for(;;)
        {
            const real delta_t = -std::log(1 - sampler.sample1().u)
                               * inv_max_density_;
            t += delta_t;
            if(t >= t_max)
                break;

            const FVec3 pos = lerp(a, b, t / t_max);
            const FVec3 unit_pos = local_to_world_.apply_inverse_to_point(pos);
            const real density = density_->sample_real(unit_pos);
            result *= 1 - density / max_density_;
        }

        return FSpectrum(result);
    }

    FSpectrum ab(
        const FVec3 &a, const FVec3 &b, Sampler &sampler) const noexcept override
    {
        // FIXME
        return tr(a, b, sampler);
    }

    SampleOutScatteringResult sample_scattering(
        const FVec3 &a, const FVec3 &b,
        Sampler &sampler, Arena &arena,
        bool indirect_scattering) const noexcept override
    {
        const real t_max = distance(a, b);
        real t = 0;

        for(;;)
        {
            const real delta_t = -std::log(1 - sampler.sample1().u)
                               * inv_max_density_;
            t += delta_t;
            if(t >= t_max)
                break;

            const FVec3 pos = lerp(a, b, t / t_max);
            const FVec3 unit_pos = local_to_world_.apply_inverse_to_point(pos);
            const real density = density_->sample_real(unit_pos);
            if(sampler.sample1().u < density / max_density_)
            {
                const FSpectrum albedo =
                    white_for_indiect_ && indirect_scattering ?
                    FSpectrum(1) : albedo_->sample_spectrum(unit_pos);
                const real     g      = g_->sample_real(unit_pos);

                MediumScattering scattering;
                scattering.pos    = pos;
                scattering.medium = this;
                scattering.wr     = (a - b) / t_max;

                auto phase_function =
                    arena.create<HenyeyGreensteinPhaseFunction>(g, albedo);

                return SampleOutScatteringResult(
                    scattering, albedo, phase_function);
            }
        }

        return SampleOutScatteringResult({}, FSpectrum(1), nullptr);
    }
};

RC<Medium> create_heterogeneous_medium(
    const FTransform3 &local_to_world,
    RC<const Texture3D> density,
    RC<const Texture3D> albedo,
    RC<const Texture3D> g,
    int max_scattering_count,
    bool white_for_indirect)
{
    return newRC<HeterogeneousMedium>(
        local_to_world, std::move(density),
        std::move(albedo), std::move(g),
        max_scattering_count, white_for_indirect);
}

AGZ_TRACER_END
