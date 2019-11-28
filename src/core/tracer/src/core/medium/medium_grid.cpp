#include <agz/tracer/core/medium.h>
#include <agz/tracer/utility/phase_function.h>
#include <agz/utility/misc.h>
#include <agz/utility/texture.h>

AGZ_TRACER_BEGIN

class GridMedium : public Medium
{
    Transform3 local_to_world_; // 从[0, 1]^3到世界坐标系的变换

    texture::texture3d_t<real> density_; // 密度图
    float sigma_a_;                      // 吸收系数
    float sigma_s_;                      // 散射系数
    real g_ = 0;                         // 散射不对称因子

    float sigma_t_;    // sigma_a_ + sigma_s_
    float maxDensity_; // density_中的最大值

    real get_density(int x, int y, int z) const noexcept
    {
        if(x < 0 || x >= density_.width() ||
           y < 0 || y >= density_.height() ||
           z < 0 || z >= density_.depth())
            return 0;
        return density_(z, y, x);
    }

    // 采样p处的三维纹理值 (p \in [0, 1]^3)
    real sample_density_in_unit(const Vec3 &p_in_unit) const
    {
        Vec3 pg = {
            p_in_unit.x * density_.width()  - real(0.5),
            p_in_unit.y * density_.height() - real(0.5),
            p_in_unit.z * density_.depth()  - real(0.5),
        };

        Vec3i pi = {
            int(std::floor(pg.x)),
            int(std::floor(pg.y)),
            int(std::floor(pg.z))
        };

        Vec3 pr = {
            pg.x - pi.x,
            pg.y - pi.y,
            pg.z - pi.z
        };

        real s00 = math::lerp(get_density(pi.x, pi.y, pi.z),
                              get_density(pi.x + 1, pi.y, pi.z), pr.x);
        real s10 = math::lerp(get_density(pi.x, pi.y + 1, pi.z),
                              get_density(pi.x + 1, pi.y + 1, pi.z), pr.x);
        real s0  = math::lerp(s00, s10, pr.y);

        real s01 = math::lerp(get_density(pi.x, pi.y, pi.z + 1),
                              get_density(pi.x + 1, pi.y, pi.z + 1), pr.x);
        real s11 = math::lerp(get_density(pi.x, pi.y + 1, pi.z + 1),
                              get_density(pi.x + 1, pi.y + 1, pi.z + 1), pr.x);
        real s1  = math::lerp(s01, s11, pr.y);

        return math::lerp(s0, s1, pr.z);
    }

    // 取得世界坐标系中p处的density
    real sample_density(const Vec3 &p) const
    {
        Vec3 unit_position = local_to_world_.apply_inverse_to_point(p);
        return sample_density_in_unit(unit_position);
    }

public:

    GridMedium(
        const Transform3 &local_to_world,
        texture::texture3d_t<real> density,
        float sigma_a, float sigma_s, real g)
    {
        local_to_world_ = local_to_world;

        density_ = std::move(density);
        sigma_a_ = sigma_a;
        sigma_s_ = sigma_s;
        g_ = g;

        sigma_t_ = sigma_s_ + sigma_a_;
        maxDensity_ = 0;
        for(int z = 0; z < density_.depth(); ++z)
        {
            for(int y = 0; y < density_.height(); ++y)
            {
                for(int x = 0; x < density_.width(); ++x)
                    maxDensity_ = (std::max)(maxDensity_, density_(z, y, x));
            }
        }
        if(maxDensity_ <= 0)
        {
            throw ObjectConstructionException(
                "invalid grid density value (max value = " + std::to_string(maxDensity_) + ")");
        }
    }

    Spectrum tr(const Vec3 &a, const Vec3 &b) const noexcept override
    {
        // TODO
        return Spectrum(1);
    }

    SampleOutScatteringResult sample_scattering(const Vec3 &a, const Vec3 &b, Sampler &sampler) const noexcept override
    {
        // TODO
        return {};
    }

    ShadingPoint shade(const MediumScattering &inct, Arena &arena) const noexcept override
    {
        return { nullptr, {} };
    }
};

std::shared_ptr<Medium> create_grid_medium(
    const Transform3 &local_to_world,
    texture::texture3d_t<real> density,
    real sigma_a, real sigma_s, real g)
{
    return std::make_shared<GridMedium>(
        local_to_world, std::move(density),
        sigma_a, sigma_s, g);
}

AGZ_TRACER_END
