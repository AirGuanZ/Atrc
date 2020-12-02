#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/utility/reflection.h>

#include "./component/aggregate.h"
#include "./component/component.h"

AGZ_TRACER_BEGIN

namespace
{
    /*
        precomputed normalization factor

        algo (in python3):

            import math
            import random
            
            N = 1000000
            
            for n in range(1, 31):
                sum = 0
                for i in range(N):
                    # sample theta_o
                    u = random.random() * math.pi / 2
                    # \int(1 - sin_theta_h)d\omega_o
                    sum += pow(1 - math.sin(0.5 * u), n) * math.cos(u)
                print(n, sum * 2 * math.pi / N)

        Io[n] can be used to normalize BSDF[n] for n in { 1, 2, ..., 30 }
    */
    constexpr real Io[] = {
        real(1),
        real(2.8940486842194333),
        real(2.2222280639882688),
        real(1.7799811477539214),
        real(1.4782676746300139),
        real(1.2586055052059806),
        real(1.091160038064419),
        real(0.9663610057614532),
        real(0.863513773340171),
        real(0.7814109559340429),
        real(0.7137752648620034),
        real(0.6541324313325798),
        real(0.6055742415342557),
        real(0.5655469687813609),
        real(0.5282206487797431),
        real(0.495548970716811),
        real(0.46634257270295826),
        real(0.44320020661126064),
        real(0.4203041901821831),
        real(0.3950607574408712),
        real(0.3797362155641827),
        real(0.36018559294675406),
        real(0.3448490864133722),
        real(0.3314408633470552),
        real(0.3184121551593867),
        real(0.3059846179205859),
        real(0.29343499125533884),
        real(0.28337741041333286),
        real(0.2748070518482051),
        real(0.26459726964555563),
        real(0.2573839594563025)
    };

    static_assert(array_size(Io) == 31);

    // see https://research.dreamworks.com/wp-content/uploads/2018/07/38-0045-deshmukh-Edited.pdf
    class FabricBSDFComponent : public BSDFComponent
    {
        FSpectrum color_;

        int n_;

    public:

        FabricBSDFComponent(const FSpectrum &color, real roughness) noexcept
            : BSDFComponent(BSDF_SPECULAR), color_(color)
        {
            n_ = static_cast<int>(std::ceil(
                1 + 29 * (1 - roughness) * (1 - roughness)));
            assert(1 <= n_ && n_ <= 30);
        }

        FSpectrum eval(
            const FVec3 &lwi, const FVec3 &lwo,
            TransMode mode) const noexcept override
        {
            if(lwi.z <= 0 || lwo.z <= 0)
                return {};

            const FVec3 lwh = (lwi + lwo).normalize();
            const real S = std::pow(1 - std::abs(lwh.x), static_cast<real>(n_));

            return color_ * S / Io[n_];
        }

        SampleResult sample(
            const FVec3 &lwo, TransMode mode,
            const Sample2 &sam) const noexcept override
        {
            if(lwo.z <= 0)
                return { {}, {}, 0 };

            // sample theta_h & phi_h

            real sign_sin_theta_h;
            real new_sam_u;

            if(sam.u < real(0.5))
            {
                sign_sin_theta_h = 1;
                new_sam_u = 2 * sam.u;
            }
            else
            {
                sign_sin_theta_h = -1;
                new_sam_u = 2 * sam.u - 1;
            }

            const real sin_theta_h = sign_sin_theta_h * math::saturate(
                1 - std::pow(new_sam_u, 1 / real(n_ + 1)));
            const real cos_theta_h = std::sqrt(1 - math::sqr(sin_theta_h));

            const real phi_h = sam.v * PI_r;

            // compute lwh & wi

            const FVec3 lwh = {
                sin_theta_h,
                cos_theta_h * std::cos(phi_h),
                cos_theta_h * std::sin(phi_h)
            };

            const FVec3 lwi = refl_aux::reflect(lwo, lwh);
            if(lwi.z <= 0)
                return {};

            // construct result

            const real S = std::pow(
                1 - std::abs(sin_theta_h), static_cast<real>(n_));

            SampleResult ret;
            ret.f   = color_ * S / Io[n_];
            ret.lwi = lwi;
            ret.pdf = real(n_ + 1) * S / (8 * PI_r * dot(lwi, lwh));

            return ret;
        }

        real pdf(const FVec3 &lwi, const FVec3 &lwo) const noexcept override
        {
            if(lwi.z <= 0 || lwo.z <= 0)
                return 0;

            const FVec3 lwh = (lwi + lwo).normalize();
            const real sin_theta_h = lwh.x;

            const real S = std::pow(
                1 - std::abs(sin_theta_h), static_cast<real>(n_));

            return real(n_ + 1) * S / (8 * PI_r * dot(lwi, lwh));
        }
    };
}

class DreamWorksFabric : public Material
{
    RC<const Texture2D> color_;
    RC<const Texture2D> roughness_;

    Box<const NormalMapper> normal_mapper_;

public:

    DreamWorksFabric(
        RC<const Texture2D> color,
        RC<const Texture2D> roughness,
        Box<const NormalMapper> normal_mapper) noexcept
        : color_(std::move(color)), roughness_(std::move(roughness)),
          normal_mapper_(std::move(normal_mapper))
    {
        
    }

    ShadingPoint shade(
        const EntityIntersection &inct, Arena &arena) const override
    {
        const FCoord shading_coord = normal_mapper_->reorient(
            inct.uv, inct.user_coord);

        const FSpectrum color = color_->sample_spectrum(inct.uv);
        const real roughness = math::saturate(roughness_->sample_real(inct.uv));

        const auto bsdf = arena.create_nodestruct<AggregateBSDF<1>>(
            inct.geometry_coord, shading_coord, color);
        bsdf->add_component(1, arena.create<FabricBSDFComponent>(
            color, roughness));

        ShadingPoint shd;
        shd.bsdf           = bsdf;
        shd.shading_normal = shading_coord.z;
        return shd;
    }
};

RC<Material> create_dream_works_fabric(
    RC<const Texture2D> color,
    RC<const Texture2D> roughness,
    Box<const NormalMapper> normal_mapper)
{
    return newRC<DreamWorksFabric>(
        std::move(color), std::move(roughness), std::move(normal_mapper));
}

AGZ_TRACER_END
