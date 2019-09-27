#include <agz/tracer/core/light.h>
#include <agz/tracer/core/texture.h>
#include <agz/utility/texture.h>

#include "./env_sampler.h"

AGZ_TRACER_BEGIN

class IBL : public EnvirLight
{
    const Texture *tex_ = nullptr;
    Vec3 up_ = Vec3(0, 0, 1);

    std::unique_ptr<EnvironmentLightSampler> sampler_;

public:

    explicit IBL(CustomedFlag customed_flag = DEFAULT_CUSTOMED_FLAG)
    {
        object_customed_flag_ = customed_flag;
    }

    static std::string description()
    {
        return R"___(
env [EnvironmentLight]
    tex [Spectrum] environment texture
    up  [Vec3]     (optional; defaultly set to +z) up direction
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        init_customed_flag(params);

        auto tex = TextureFactory.create(params.child_group("tex"), init_ctx);

        Vec3 up(0, 0, 1);
        if(params.find_child("up"))
            up = params.child_vec3("up");

        initialize(tex, up);

        AGZ_HIERARCHY_WRAP("in initializing native sky light")
    }

    void initialize(const Texture *tex, const Vec3 &up)
    {
        AGZ_HIERARCHY_TRY

        tex_ = tex;
        up_ = up;
        sampler_ = std::make_unique<EnvironmentLightSampler>(tex_);

        AGZ_HIERARCHY_WRAP("in initializing native sky light")
    }

    EnvirLightSampleResult sample(const Vec3 &ref, const Sample3 &sam) const noexcept override
    {
        auto [dir, pdf] = sampler_->sample(sam);

        EnvirLightSampleResult ret;
        ret.ref_to_light = dir;
        ret.radiance = radiance(ref, dir);
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

    Spectrum radiance(const Vec3 &ref, const Vec3 &ref_to_light) const noexcept override
    {
        Vec3 dir = Coord::from_z(up_).global_to_local(ref_to_light).normalize();
        real phi = local_angle::phi(dir);
        real theta = local_angle::theta(dir);
        real u = math::clamp<real>(phi / (2 * PI_r), 0, 1);
        real v = math::clamp<real>(theta / PI_r, 0, 1);
        return tex_->sample_spectrum({ u, v });
    }
};

EnvirLight *create_ibl_light(
    const Texture *tex,
    const Vec3 &up,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena)
{
    auto ret = arena.create<IBL>(customed_flag);
    ret->initialize(tex, up);
    return ret;
}

AGZT_IMPLEMENTATION(EnvirLight, IBL, "ibl")

// 为了前向兼容
using IBL2 = IBL;
AGZT_IMPLEMENTATION(EnvirLight, IBL2, "env")

AGZ_TRACER_END
