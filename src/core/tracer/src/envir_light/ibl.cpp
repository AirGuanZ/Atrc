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

        tex_ = TextureFactory.create(params.child_group("tex"), init_ctx);
        if(params.find_child("up"))
            up_ = params.child_vec3("up");

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
    
    //LightEmitResult emit(const InfiniteLightCore &core, const Sample4 &sam) const noexcept override
    //{
    //    LightEmitResult ret;

    //    auto [unit_pos, unit_pos_pdf] = math::distribution::uniform_on_sphere(sam.u, sam.v);
    //    ret.spt = core.unit_pos_to_spt(unit_pos);
    //    ret.pdf_pos = unit_pos_pdf / (core.world_radius() * core.world_radius());

    //    auto [local_dir, dir_pdf] = math::distribution::zweighted_on_hemisphere(sam.w, sam.r);
    //    auto global_dir = ret.spt.geometry_coord.local_to_global(local_dir);
    //    ret.dir = global_dir;
    //    ret.pdf_dir = dir_pdf;
    //    
    //    ret.radiance = radiance(-global_dir);

    //    return ret;
    //}

    //void emit_pdf(const InfiniteLightCore &core, const SurfacePoint &spt, const Vec3 &light_to_out,
    //              real *pdf_pos, real *pdf_dir) const noexcept override
    //{
    //    real z = cos(light_to_out, spt.geometry_coord.z);
    //    *pdf_pos = math::distribution::uniform_on_sphere_pdf<real> / (core.world_radius() * core.world_radius());
    //    *pdf_dir = math::distribution::zweighted_on_hemisphere_pdf(z);
    //}
};

AGZT_IMPLEMENTATION(EnvirLight, IBL, "ibl")

// 为了前向兼容
using IBL2 = IBL;
AGZT_IMPLEMENTATION(EnvirLight, IBL2, "env")

AGZ_TRACER_END
