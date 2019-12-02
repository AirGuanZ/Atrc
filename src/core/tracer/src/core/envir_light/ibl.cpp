#include <agz/tracer/core/light.h>
#include <agz/tracer/core/texture2d.h>
#include <agz/utility/misc.h>
#include <agz/utility/texture.h>

#include "./env_sampler.h"

AGZ_TRACER_BEGIN

class IBL : public EnvirLight
{
    std::shared_ptr<const Texture2D> tex_;
    Vec3 up_ = Vec3(0, 0, 1);

    std::unique_ptr<EnvironmentLightSampler> sampler_;

public:

    void initialize(std::shared_ptr<const Texture2D> tex, const Vec3 &up)
    {
        AGZ_HIERARCHY_TRY

        tex_ = tex;
        up_ = up;
        sampler_ = std::make_unique<EnvironmentLightSampler>(tex_);

        AGZ_HIERARCHY_WRAP("in initializing native sky light")
    }

    LightSampleResult sample(const Vec3 &ref, const Sample5 &sam) const noexcept override
    {
        auto [dir, pdf] = sampler_->sample({ sam.u, sam.v, sam.w });

        LightSampleResult ret;
        ret.ref      = ref;
        ret.pos      = ref + 4 * world_radius_ * dir;
        ret.radiance = radiance(ref, dir, nullptr);
        ret.pdf      = pdf;
        ret.is_delta = false;

        return ret;
    }

    real pdf(const Vec3 &ref, const Vec3 &ref_to_light) const noexcept override
    {
        return sampler_->pdf(ref_to_light);
    }

    Spectrum power() const noexcept override
    {
        real radius = world_radius_;
        Spectrum ret;

        int tex_width = tex_->width(), tex_height = tex_->height();
        for(int y = 0; y < tex_height; ++y)
        {
            real v0 = real(y) / tex_height;
            real v1 = real(y + 1) / tex_height;

            for(int x = 0; x < tex_width; ++x)
            {
                real u0 = real(x) / tex_width;
                real u1 = real(x + 1) / tex_width;

                real delta_area = std::abs(2 * PI_r * (u1 - u0) * (std::cos(PI_r * v1) - std::cos(PI_r * v0)));
                real u = (u0 + u1) / 2, v = (v0 + v1) / 2;
                ret += PI_r * delta_area * tex_->sample_spectrum({ u, v });
            }
        }

        return ret * radius * radius;
    }

    Spectrum radiance(const Vec3 &ref, const Vec3 &ref_to_light, Vec3 *light_pnt) const noexcept override
    {
        if(light_pnt)
            *light_pnt = ref + 4 * world_radius_ * ref_to_light.normalize();
        Vec3 dir = Coord::from_z(up_).global_to_local(ref_to_light).normalize();
        real phi = local_angle::phi(dir);
        real theta = local_angle::theta(dir);
        real u = math::clamp<real>(phi / (2 * PI_r), 0, 1);
        real v = math::clamp<real>(theta / PI_r, 0, 1);
        return tex_->sample_spectrum({ u, v });
    }
};

std::shared_ptr<NonareaLight>create_ibl_light(
    std::shared_ptr<const Texture2D> tex,
    const Vec3 &up)
{
    auto ret = std::make_shared<IBL>();
    ret->initialize(tex, up);
    return ret;
}

AGZ_TRACER_END
