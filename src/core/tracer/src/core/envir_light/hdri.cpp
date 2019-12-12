#include <agz/tracer/core/light.h>
#include <agz/tracer/core/texture2d.h>
#include <agz/utility/misc.h>
#include <agz/utility/texture.h>

#include "./env_sampler.h"

AGZ_TRACER_BEGIN

class HDRI : public EnvirLight
{
    std::shared_ptr<const Texture2D> tex_;
    Vec3 up_ = Vec3(0, 0, 1);

    std::unique_ptr<EnvironmentLightSampler> sampler_;

    real radius_ = 100;
    Vec3 offset_;

    Vec3 get_npos(const Vec3 &o, const Vec3 &d) const
    {
        Vec3 local_o = o - offset_;
        real A = dot(d, d);
        real B = 2 * dot(d, local_o);
        real C = dot(local_o, local_o) - radius_ * radius_;
        real delta = B * B - 4 * A * C;
        if(delta < 0)
            return Vec3(0);
        real t = (-B + std::sqrt(delta)) / (2 * A);
        if(t < EPS)
            return Vec3(0);
        return (local_o + t * d).normalize();
    }

public:

    void initialize(std::shared_ptr<const Texture2D> tex, const Vec3 &up, real radius, const Vec3 &offset)
    {
        AGZ_HIERARCHY_TRY

        tex_ = tex;
        sampler_ = std::make_unique<EnvironmentLightSampler>(tex_);

        up_ = up;
        
        radius_ = radius;
        if(radius <= 0)
            throw ObjectConstructionException("invalid radius value: " + std::to_string(radius_));

        offset_ = offset;

        AGZ_HIERARCHY_WRAP("in initializing hdri environment light")
    }

    LightSampleResult sample(const Vec3 &ref, const Sample5 &sam) const noexcept override
    {
        auto [npos, pdf_area] = sampler_->sample({ sam.u, sam.v, sam.w });
        Vec3 pos = npos * radius_ + offset_;

        Vec3 ref_to_light = (pos - ref).normalize();

        LightSampleResult ret;
        ret.ref          = ref;
        ret.pos          = pos;
        ret.radiance     = radiance(ref, ref_to_light, nullptr);
        ret.pdf          = pdf_area / (radius_ * radius_) * (pos - ref).length_square() / std::abs(dot(ref_to_light, npos));
        ret.is_delta     = false;

        return ret;
    }

    real pdf(const Vec3 &ref, const Vec3 &ref_to_light) const noexcept override
    {
        Vec3 npos = get_npos(ref, ref_to_light);
        if(!npos)
            return 0;
        Vec3 pos = npos * radius_ + offset_;
        real pdf_area = sampler_->pdf(npos);
        return pdf_area / (radius_ * radius_) * (pos - ref).length_square() / std::abs(cos(ref_to_light, npos));
    }

    LightEmitResult emit(const Sample5 &sam) const noexcept override
    {
        // TODO
        return {};
    }

    LightEmitPDFResult emit_pdf(const Vec3 &position, const Vec3 &direction, const Vec3 &normal) const noexcept override
    {
        // TODO
        return {};
    }

    Spectrum power() const noexcept override
    {
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

        return ret * radius_ * radius_;
    }

    Spectrum radiance(const Vec3 &ref, const Vec3 &ref_to_light, Vec3 *light_pnt) const noexcept override
    {
        Vec3 npos = get_npos(ref, ref_to_light);
        if(!npos)
            return {};

        if(light_pnt)
            *light_pnt = ref + 4 * world_radius_ * ref_to_light.normalize();

        real phi = local_angle::phi(npos);
        real theta = local_angle::theta(npos);
        real u = math::clamp<real>(phi / (2 * PI_r), 0, 1);
        real v = math::clamp<real>(theta / PI_r, 0, 1);
        return tex_->sample_spectrum({ u, v });
    }

    void preprocess(const Scene &scene) override
    {
        EnvirLight::preprocess(scene);

        // hdri球体应包含整个场景的包围球，否则报错
        if(distance(world_centre_, offset_) + world_radius_ > radius_)
            throw ObjectConstructionException("invalid hdri offset/radius: cannot enclose scene");
    }
};

std::shared_ptr<NonareaLight> create_hdri_light(
    std::shared_ptr<const Texture2D> tex,
    const Vec3 &up,
    real radius,
    const Vec3 &offset)
{
    auto ret = std::make_shared<HDRI>();
    ret->initialize(tex, up, radius, offset);
    return ret;
}

AGZ_TRACER_END
