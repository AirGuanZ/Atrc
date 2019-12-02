#include <agz/tracer/core/medium.h>
#include <agz/tracer/core/texture3d.h>
#include <agz/tracer/utility/phase_function.h>
#include <agz/utility/misc.h>
#include <agz/utility/texture.h>

AGZ_TRACER_BEGIN

class GridMedium : public Medium
{
    Transform3 local_to_world_;          // 从[0, 1]^3到世界坐标系的变换

    real sigma_a_;                      // 吸收系数
    real sigma_s_;                      // 散射系数
    real g_ = 0;                         // 散射不对称因子

    real sigma_t_;                      // sigma_a_ + sigma_s_

    std::shared_ptr<const Texture3D> density_;
    real                             max_density_;

    // 取得世界坐标系中p处的density
    real sample_density(const Vec3 &p) const
    {
        Vec3 unit_position = local_to_world_.apply_inverse_to_point(p);
        return density_->sample_real(unit_position);
    }

public:

    GridMedium(
        const Transform3 &local_to_world,
        std::shared_ptr<const Texture3D> density,
        float sigma_a, float sigma_s, real g)
    {
        local_to_world_ = local_to_world;

        sigma_a_ = sigma_a;
        sigma_s_ = sigma_s;
        g_ = g;

        sigma_t_ = sigma_s_ + sigma_a_;

        density_     = std::move(density);
        max_density_ = density_->max_real();
        if(max_density_ <= 0)
        {
            throw ObjectConstructionException(
                "invalid grid density value (max value = " + std::to_string(max_density_) + ")");
        }
    }

    Spectrum tr(const Vec3 &a, const Vec3 &b, Sampler &sampler) const noexcept override
    {
        real result = 1;
        real t_max = distance(a, b), t = 0;
        real sample_ratio = 1 / (max_density_ * sigma_t_);

        for(;;)
        {
            real delta_t = -std::log(1 - sampler.sample1().u) * sample_ratio;
            t += delta_t;
            if(t >= t_max)
                break;

            Vec3 pos = lerp(a, b, t / t_max);
            real density = sample_density(pos);
            result *= 1 - density / max_density_;
        }

        return Spectrum(result);
    }

    SampleOutScatteringResult sample_scattering(const Vec3 &a, const Vec3 &b, Sampler &sampler, Arena &arena) const noexcept override
    {
        real t_max = distance(a, b), t = 0;
        real sample_ratio = 1 / (max_density_ * sigma_t_);

        for(;;)
        {
            real delta_t = -std::log(1 - sampler.sample1().u) * sample_ratio;
            t += delta_t;
            if(t >= t_max)
                break;

            Vec3 pos = lerp(a, b, t / t_max);
            real density = sample_density(pos);
            if(sampler.sample1().u < density / max_density_)
            {
                real albedo = sigma_t_ > 0 ? sigma_s_ / sigma_t_ : 1;
                
                MediumScattering scattering;
                scattering.pos    = pos;
                scattering.medium = this;
                scattering.wr     = (a - b) / t_max;

                SampleOutScatteringResult result;
                result.scattering_point = scattering;
                result.throughput       = Spectrum(albedo);

                result.phase_function = arena.create<HenyeyGreensteinPhaseFunction>(g_, Spectrum(albedo));

                return result;
            }
        }

        SampleOutScatteringResult result;
        result.throughput = Spectrum(1);
        return result;
    }
};

std::shared_ptr<Medium> create_grid_medium(
    const Transform3 &local_to_world,
    std::shared_ptr<const Texture3D> density,
    real sigma_a, real sigma_s, real g)
{
    return std::make_shared<GridMedium>(
        local_to_world, std::move(density),
        sigma_a, sigma_s, g);
}

AGZ_TRACER_END
