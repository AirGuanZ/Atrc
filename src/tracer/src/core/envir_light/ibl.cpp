#include <agz/tracer/core/light.h>
#include <agz/tracer/core/texture2d.h>
#include <agz/tracer/create/texture2d.h>
#include <agz/utility/misc.h>
#include <agz/utility/texture.h>

#include "./env_sampler.h"

AGZ_TRACER_BEGIN

class IBL : public EnvirLight
{
    RC<const Texture2D> tex_;

    FSpectrum avg_rad_;

    Box<EnvironmentLightSampler> sampler_;

    real user_specified_power_;

public:

    IBL(
        RC<const Texture2D> tex,
        bool no_importance_sampling,
        real user_specified_power = -1)
    {
        tex_ = tex;
        user_specified_power_ = user_specified_power;

        if(no_importance_sampling)
            sampler_ = newBox<EnvironmentLightSampler>(
                create_constant2d_texture({}, FSpectrum(1)));
        else
            sampler_ = newBox<EnvironmentLightSampler>(tex_);

        if(no_importance_sampling)
        {
            avg_rad_ = FSpectrum(1);
        }
        else
        {
            const int tex_width = tex_->width(), tex_height = tex_->height();
            for(int y = 0; y < tex_height; ++y)
            {
                const real v0 = real(y) / tex_height;
                const real v1 = real(y + 1) / tex_height;

                for(int x = 0; x < tex_width; ++x)
                {
                    const real u0 = real(x) / tex_width;
                    const real u1 = real(x + 1) / tex_width;

                    const real delta_area = std::abs(2 * PI_r * (u1 - u0) *
                                            (std::cos(PI_r * v1) - std::cos(PI_r * v0)));

                    const real u = (u0 + u1) / 2, v = (v0 + v1) / 2;
                    avg_rad_ += PI_r * delta_area * tex_->sample_spectrum({ u, v });
                }
            }
        }
    }

    LightSampleResult sample(
        const FVec3 &ref, const Sample5 &sam) const noexcept override
    {
        const auto [dir, pdf] = sampler_->sample({ sam.u, sam.v, sam.w });

        return LightSampleResult(
            ref, emit_pos(ref, dir).pos, -dir, radiance(ref, dir), pdf);
    }

    real pdf(const FVec3 &ref, const FVec3 &ref_to_light) const noexcept override
    {
        return sampler_->pdf(ref_to_light);
    }

    LightEmitResult sample_emit(const Sample5 &sam) const noexcept override
    {
        auto [dir, pdf_dir] = sampler_->sample({ sam.u, sam.v, sam.w });
        dir = -dir;

        const Vec2 disk_sam = math::distribution
                                ::uniform_on_unit_disk(sam.r, sam.s);
        const FCoord dir_coord = FCoord::from_z(dir);
        
        const FVec3 pos = world_centre_ +
            (disk_sam.x * dir_coord.x + disk_sam.y * dir_coord.y - dir) * world_radius_;

        return LightEmitResult(
            pos, dir, dir.normalize(), {}, radiance(world_centre_, -dir),
            1 / (PI_r * world_radius_ * world_radius_), pdf_dir);
    }

    LightEmitPDFResult emit_pdf(
        const FVec3 &pos, const FVec3 &dir, const FVec3 &nor) const noexcept override
    {
        const real pdf_pos = 1 / (PI_r * world_radius_ * world_radius_);
        const real pdf_dir = sampler_->pdf(-dir);
        return { pdf_pos, pdf_dir };
    }

    LightEmitPosResult emit_pos(
        const FVec3 &ref, const FVec3 &ref_to_light) const noexcept override
    {
        // o: world_center
        // r: world_radius
        // x: ref
        // d: ref_to_light.normalize()
        // o + r * (u * ex + v * ey + d) = x + d * t
        // solve [u, v, t] and ans = ref + ref_to_light * t

        // => [a b c][u v t]^T = m
        // where a = r * ex
        //       b = r * ey
        //       c = -d
        //       m = x - o + r * d

        const auto [ex, ey, d] = FCoord::from_z(ref_to_light);

        const FVec3 a = world_radius_ * ex;
        const FVec3 b = world_radius_ * ey;
        const FVec3 c = -d;
        const FVec3 m = ref - world_centre_ - world_radius_ * d;

        const real det  = Mat3::from_cols(a, b, c).det();
        const real tdet = Mat3::from_cols(a, b, m).det();

        if(std::abs(det) < EPS())
            return { world_centre_, -ref_to_light };

        const real t = tdet / det;
        const FVec3 pos = ref + t * d;

        return { pos, c };
    }

    FSpectrum power() const noexcept override
    {
        const real radius = world_radius_;
        return user_specified_power_ > 0 ? FSpectrum(user_specified_power_)
                                         : avg_rad_ * radius * radius;
    }

    FSpectrum radiance(
        const FVec3 &ref, const FVec3 &ref_to_light) const noexcept override
    {
        const FVec3 dir   = ref_to_light.normalize();
        const real phi   = local_angle::phi(dir);
        const real theta = local_angle::theta(dir);
        const real u     = math::clamp<real>(phi / (2 * PI_r), 0, 1);
        const real v     = math::clamp<real>(theta / PI_r, 0, 1);
        return tex_->sample_spectrum({ u, v });
    }
};

RC<EnvirLight> create_ibl_light(
    RC<const Texture2D> tex,
    bool no_importance_sampling,
    real user_specified_power)
{
    return newRC<IBL>(tex, no_importance_sampling, user_specified_power);
}

AGZ_TRACER_END
