#pragma once

#include <agz/tracer/core/texture.h>
#include <agz/utility/texture.h>

#include "./infinite_light.h"
#include <random>

AGZ_TRACER_BEGIN

namespace env_impl
{

    class EnvironmentLightSampler : public misc::uncopyable_t
    {
        texture::texture2d_t<real> probs_;
        math::distribution::alias_sampler_t<real> sampler_;

    public:

        explicit EnvironmentLightSampler(const Texture *tex)
        {
            int width = tex->width(), height = tex->height();
            int new_width = (std::min)(width, 1);
            int new_height = (std::min)(height, 1);

            // 得到能量分布图
            real lum_sum = 0;
            probs_.initialize(new_height, new_width);
            for(int y = 0; y < new_height; ++y)
            {
                real y0 = real(y)     / new_height;
                real y1 = real(y + 1) / new_height;

                real y0_src = y0 * height;
                real y1_src = y1 * height;

                int y_src_beg = (std::max)(0,                int(std::floor(y0_src) - 1));
                int y_src_lst = (std::min)(height - 1, int(std::floor(y1_src) + 1));

                for(int x = 0; x < new_width; ++x)
                {
                    real x0 = real(x)     / new_width;
                    real x1 = real(x + 1) / new_width;

                    real x0_src = x0 * width;
                    real x1_src = x1 * width;

                    int x_src_beg = (std::max)(0,               int(std::floor(x0_src) - 1));
                    int x_src_lst = (std::min)(width - 1, int(std::floor(x1_src) + 1));

                    real pixel_lum = 0;
                    for(int y_src = y_src_beg; y_src <= y_src_lst; ++y_src)
                    {
                        for(int x_src = x_src_beg; x_src <= x_src_lst; ++x_src)
                            pixel_lum += tex->fetch_spectrum(x_src, y_src).lum();
                    }

                    real delta_area = std::abs(2 * PI_r * (x1 - x0) * (std::cos(PI_r * y1) - std::cos(PI_r * y0)));
                    real area_lum = pixel_lum * delta_area;
                    probs_(y, x) = area_lum;
                    lum_sum += area_lum;
                }
            }

            // 归一化
            if(lum_sum > EPS)
            {
                real ratio = 1 / lum_sum;
                for(int y = 0; y < new_height; ++y)
                {
                    for(int x = 0; x < new_width; ++x)
                        probs_(y, x) *= ratio;
                }
            }

            // 线性化
            std::vector<real> linear_probs(probs_.size().product());
            for(int y = 0; y < new_height; ++y)
            {
                int y_base = y * new_width;
                for(int x = 0; x < new_width; ++x)
                {
                    int idx = y_base + x;
                    linear_probs[idx] = probs_(y, x);
                }
            }

            // 构造sampler
            sampler_.initialize(linear_probs.data(), int(linear_probs.size()));
        }

        // return (ref_to_light, pdf)
        std::pair<Vec3, real> sample(const Sample3 &sam) const
        {
            int patch_idx = sampler_.sample(sam.u);
            int patch_y = patch_idx / probs_.width();
            int patch_x = patch_idx & probs_.width();
            assert(patch_y < probs_.height());
            assert(patch_x < probs_.width());

            real patch_pdf = probs_(patch_y, patch_x);

            real u0 = real(patch_x)     / probs_.width();
            real u1 = real(patch_x + 1) / probs_.width();
            real v0 = real(patch_y)     / probs_.height();
            real v1 = real(patch_y + 1) / probs_.height();

            auto [cvmin, cvmax] = std::minmax(std::cos(PI_r * v1), std::cos(PI_r * v0));

            real cos_theta = cvmin + sam.w * (cvmax - cvmin);
            real sin_theta = local_angle::cos_2_sin(cos_theta);
            real u = math::mix(u0, u1, sam.v);
            real phi = 2 * PI_r * u;

            Vec3 dir(sin_theta * std::cos(phi), sin_theta * std::sin(phi), cos_theta);
            real in_patch_pdf = 1 / (2 * PI_r * ((u1 - u0) * (cvmax - cvmin)));

            return { dir, patch_pdf * in_patch_pdf };
        }

        real pdf(const Vec3 &ref_to_light) const
        {
            Vec3 dir = ref_to_light.normalize();
            real cos_theta = local_angle::cos_theta(dir);
            real theta     = std::acos(cos_theta);
            real phi       = local_angle::phi(dir);

            real u = phi / (2 * PI_r);
            real v = theta / PI_r;

            int patch_x = (std::min)(probs_.width() - 1,  int(std::floor(u * probs_.width())));
            int patch_y = (std::min)(probs_.height() - 1, int(std::floor(v * probs_.height())));
            real patch_pdf = probs_(patch_y, patch_x);

            real u0 = real(patch_x)     / probs_.width();
            real u1 = real(patch_x + 1) / probs_.width();
            real v0 = real(patch_y)     / probs_.height();
            real v1 = real(patch_y + 1) / probs_.height();

            auto [cvmin, cvmax] = std::minmax(std::cos(PI_r * v1), std::cos(PI_r * v0));
            real in_patch_pdf = 1 / (2 * PI_r * ((u1 - u0) * (cvmax - cvmin)));

            return patch_pdf * in_patch_pdf;
        }
    };

} // namespace env_impl

// IMPROVE: add importance sampling in emitting
class EnvironmentLightImpl : public InfiniteLightImpl
{
    const Texture *tex_ = nullptr;
    Vec3 up_ = Vec3(0, 0, 1);

    std::unique_ptr<env_impl::EnvironmentLightSampler> sampler_;

    Spectrum radiance(const Vec3 &ref_to_light) const noexcept
    {
        Vec3 dir = Coord::from_z(up_).global_to_local(ref_to_light).normalize();
        real phi = local_angle::phi(dir);
        real theta = local_angle::theta(dir);
        real u = math::clamp<real>(phi / (2 * PI_r), 0, 1);
        real v = math::clamp<real>(theta / PI_r, 0, 1);
        return tex_->sample_spectrum({ u, v });
    }

public:

    static std::string description()
    {
        return R"___(
env [(Nonarea)Light]
    tex [Spectrum] environment texture
    up  [Vec3]     (optional; defaultly set to +z) up directory
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        tex_ = TextureFactory.create(params.child_group("tex"), init_ctx);
        if(params.find_child("up"))
            up_ = params.child_vec3("up");

        sampler_ = std::make_unique<env_impl::EnvironmentLightSampler>(tex_);

        AGZ_HIERARCHY_WRAP("in initializing native sky light")
    }

    LightSampleResult sample(const InfiniteLightCore &core, const Vec3 &ref, const Sample4 &sam) const noexcept override
    {
        auto [dir, pdf] = sampler_->sample({ sam.u, sam.v, sam.w });

        Ray global_r(ref, dir, EPS);
        GeometryIntersection inct;
        core.intersection(global_r, &inct);

        LightSampleResult ret;
        ret.spt      = static_cast<SurfacePoint&>(inct);
        ret.radiance = radiance(dir);
        ret.pdf      = pdf;
        ret.is_delta = false;

        return ret;
    }

    real pdf(const InfiniteLightCore &core, const Vec3&, const Vec3 &ref_to_light) const noexcept override
    {
        return sampler_->pdf(ref_to_light);
    }

    Spectrum power(const InfiniteLightCore &core) const noexcept override
    {
        real radius = core.world_radius();
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
                ret += PI_r * delta_area * tex_->fetch_spectrum(x, y);
            }
        }

        return ret * radius * radius;
    }

    Spectrum radiance(const InfiniteLightCore &, const Vec3 &ref_to_light) const noexcept override
    {
        return radiance(ref_to_light);
    }
    
    LightEmitResult emit(const InfiniteLightCore &core, const Sample4 &sam) const noexcept override
    {
        LightEmitResult ret;

        auto [unit_pos, unit_pos_pdf] = math::distribution::uniform_on_sphere(sam.u, sam.v);
        ret.spt = core.unit_pos_to_spt(unit_pos);
        ret.pdf_pos = unit_pos_pdf / (core.world_radius() * core.world_radius());

        auto [local_dir, dir_pdf] = math::distribution::zweighted_on_hemisphere(sam.w, sam.r);
        auto global_dir = ret.spt.geometry_coord.local_to_global(local_dir);
        ret.dir = global_dir;
        ret.pdf_dir = dir_pdf;
        
        ret.radiance = radiance(-global_dir);

        return ret;
    }

    void emit_pdf(const InfiniteLightCore &core, const SurfacePoint &spt, const Vec3 &light_to_out,
                  real *pdf_pos, real *pdf_dir) const noexcept override
    {
        real z = cos(light_to_out, spt.geometry_coord.z);
        *pdf_pos = math::distribution::uniform_on_sphere_pdf<real> / (core.world_radius() * core.world_radius());
        *pdf_dir = math::distribution::zweighted_on_hemisphere_pdf(z);
    }
};

AGZ_TRACER_END
