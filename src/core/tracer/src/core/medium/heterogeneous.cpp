#include <agz/tracer/core/medium.h>
#include <agz/tracer/core/texture3d.h>
#include <agz/tracer/utility/phase_function.h>
#include <agz/utility/misc.h>
#include <agz/utility/texture.h>

AGZ_TRACER_BEGIN

class HeterogeneousMedium : public Medium
{
    Transform3 local_to_world_;

    std::shared_ptr<const Texture3D> density_;
    std::shared_ptr<const Texture3D> albedo_;
    std::shared_ptr<const Texture3D> g_;

    real max_density_;
    real inv_max_density_;

public:

    HeterogeneousMedium(
        const Transform3 &local_to_world,
        std::shared_ptr<const Texture3D> density,
        std::shared_ptr<const Texture3D> albedo,
        std::shared_ptr<const Texture3D> g)
    {
        local_to_world_ = local_to_world;

        density_ = std::move(density);
        albedo_  = std::move(albedo);
        g_       = std::move(g);

        max_density_ = density_->max_real();
        if(max_density_ < EPS)
            throw ObjectConstructionException("invalid max density value: " + std::to_string(max_density_));
        inv_max_density_ = 1 / max_density_;
    }

    Spectrum tr(const Vec3 &a, const Vec3 &b, Sampler &sampler) const noexcept override
    {
        real result = 1;
        real t_max = distance(a, b), t = 0;

        for(;;)
        {
            real delta_t = -std::log(1 - sampler.sample1().u) * inv_max_density_;
            t += delta_t;
            if(t >= t_max)
                break;

            Vec3 pos = lerp(a, b, t / t_max);
            Vec3 unit_pos = local_to_world_.apply_inverse_to_point(pos);
            real density = density_->sample_real(unit_pos);
            result *= 1 - density / max_density_;
        }

        return Spectrum(result);
    }

    SampleOutScatteringResult sample_scattering(const Vec3 &a, const Vec3 &b, Sampler &sampler, Arena &arena) const noexcept override
    {
        real t_max = distance(a, b), t = 0;
        
        for(;;)
        {
            real delta_t = -std::log(1 - sampler.sample1().u) * inv_max_density_;
            t += delta_t;
            if(t >= t_max)
                break;

            Vec3 pos = lerp(a, b, t / t_max);
            Vec3 unit_pos = local_to_world_.apply_inverse_to_point(pos);
            real density = density_->sample_real(unit_pos);
            if(sampler.sample1().u < density / max_density_)
            {
                Spectrum albedo = albedo_->sample_spectrum(unit_pos);
                real     g      = g_->sample_real(unit_pos);

                MediumScattering scattering;
                scattering.pos    = pos;
                scattering.medium = this;
                scattering.wr     = (a - b) / t_max;

                SampleOutScatteringResult result;
                result.scattering_point = scattering;
                result.throughput       = Spectrum(albedo);

                result.phase_function = arena.create<HenyeyGreensteinPhaseFunction>(g, Spectrum(albedo));

                return result;
            }
        }

        SampleOutScatteringResult result;
        result.throughput = Spectrum(1);
        return result;
    }
};

std::shared_ptr<Medium> create_heterogeneous_medium(
    const Transform3 &local_to_world,
    std::shared_ptr<const Texture3D> density,
    std::shared_ptr<const Texture3D> albedo,
    std::shared_ptr<const Texture3D> g)
{
    return std::make_shared<HeterogeneousMedium>(
        local_to_world, std::move(density), std::move(albedo), std::move(g));
}

AGZ_TRACER_END
