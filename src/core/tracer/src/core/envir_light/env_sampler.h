#pragma once

#include <algorithm>
#include <vector>

#include <agz/tracer/core/texture.h>
#include <agz/utility/texture.h>

AGZ_TRACER_BEGIN

class EnvironmentLightSampler : public misc::uncopyable_t
{
    texture::texture2d_t<real> probs_;
    math::distribution::alias_sampler_t<real> sampler_;

public:

    explicit EnvironmentLightSampler(std::shared_ptr<const Texture> tex)
    {
        int width = tex->width(), height = tex->height();
        int new_width = (std::min)(width, 200);
        int new_height = (std::min)(height, 200);

        // 得到能量分布图
        real lum_sum = 0;
        probs_.initialize(new_height, new_width);
        for(int y = 0; y < new_height; ++y)
        {
            real y0 = real(y) / new_height;
            real y1 = real(y + 1) / new_height;

            real y0_src = y0 * height;
            real y1_src = y1 * height;

            int y_src_beg = (std::max)(0, int(std::floor(y0_src) - 1));
            int y_src_lst = (std::min)(height - 1, int(std::floor(y1_src) + 1));

            for(int x = 0; x < new_width; ++x)
            {
                real x0 = real(x) / new_width;
                real x1 = real(x + 1) / new_width;

                real x0_src = x0 * width;
                real x1_src = x1 * width;

                int x_src_beg = (std::max)(0, int(std::floor(x0_src) - 1));
                int x_src_lst = (std::min)(width - 1, int(std::floor(x1_src) + 1));

                real pixel_lum = 0;
                for(int y_src = y_src_beg; y_src <= y_src_lst; ++y_src)
                {
                    for(int x_src = x_src_beg; x_src <= x_src_lst; ++x_src)
                    {
                        real src_u = (x_src + real(0.5)) / width;
                        real src_v = (y_src + real(0.5)) / height;
                        pixel_lum += tex->sample_spectrum({ src_u, src_v }).lum();
                    }
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
        int patch_x = patch_idx % probs_.width();
        assert(patch_y < probs_.height());
        assert(patch_x < probs_.width());

        real patch_pdf = probs_(patch_y, patch_x);

        real u0 = real(patch_x) / probs_.width();
        real u1 = real(patch_x + 1) / probs_.width();
        real v0 = real(patch_y) / probs_.height();
        real v1 = real(patch_y + 1) / probs_.height();

        auto [cvmin, cvmax] = math::minmax(std::cos(PI_r * v1), std::cos(PI_r * v0));

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
        real theta = std::acos(cos_theta);
        real phi = local_angle::phi(dir);

        real u = phi / (2 * PI_r);
        real v = theta / PI_r;

        int patch_x = (std::min)(probs_.width() - 1, int(std::floor(u * probs_.width())));
        int patch_y = (std::min)(probs_.height() - 1, int(std::floor(v * probs_.height())));
        real patch_pdf = probs_(patch_y, patch_x);

        real u0 = real(patch_x) / probs_.width();
        real u1 = real(patch_x + 1) / probs_.width();
        real v0 = real(patch_y) / probs_.height();
        real v1 = real(patch_y + 1) / probs_.height();

        auto[cvmin, cvmax] = math::minmax(std::cos(PI_r * v1), std::cos(PI_r * v0));
        real in_patch_pdf = 1 / (2 * PI_r * ((u1 - u0) * (cvmax - cvmin)));

        return patch_pdf * in_patch_pdf;
    }
};

AGZ_TRACER_END
