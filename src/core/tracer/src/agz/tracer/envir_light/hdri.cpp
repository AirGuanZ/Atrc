#include <agz/tracer/core/light.h>
#include <agz/tracer/core/texture.h>
#include <agz/utility/texture.h>

#include "./env_sampler.h"

AGZ_TRACER_BEGIN

class HDRI : public EnvirLight
{
    const Texture *tex_ = nullptr;
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

    explicit HDRI(CustomedFlag customed_flag = DEFAULT_CUSTOMED_FLAG)
    {
        object_customed_flag_ = customed_flag;
    }

    static std::string description()
    {
        return R"___(
hdri [EnvironmentLight]
	tex    [Texture] environment mapped texture
    up     [Vec3]    (optional; defaultly set to +z) up direction
    radius [real]    (optional; defaultly set to 100) hdri sphere radius
    offset [Vec3]    (optional; defaultly set to 0) hdri sphere translation in world space
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        init_customed_flag(params);
        
        auto tex = TextureFactory.create(params.child_group("tex"), init_ctx);
        
        Vec3 up = Vec3(0, 0, 1);
        if(params.find_child("up"))
            up = params.child_vec3("up");

        real radius = params.child_real_or("radius", 100);

        Vec3 offset;
        if(params.find_child("offset"))
            offset = params.child_vec3("offset");

        initialize(tex, up, radius, offset);

        AGZ_HIERARCHY_WRAP("in initializing hdri environment light")
    }

    void initialize(const Texture *tex, const Vec3 &up, real radius, const Vec3 &offset)
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

    EnvirLightSampleResult sample(const Vec3 &ref, const Sample3 &sam) const noexcept override
    {
        auto [npos, pdf_area] = sampler_->sample(sam);
        Vec3 pos = npos * radius_ + offset_;

        EnvirLightSampleResult ret;
        ret.ref_to_light = (pos - ref).normalize();
        ret.radiance     = radiance(ref, ret.ref_to_light);
        ret.pdf          = pdf_area / (radius_ * radius_) * (pos - ref).length_square() / std::abs(dot(ret.ref_to_light, npos));
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

    Spectrum radiance(const Vec3 &ref, const Vec3 &ref_to_light) const noexcept override
    {
        Vec3 npos = get_npos(ref, ref_to_light);
        if(!npos)
            return {};
        real phi = local_angle::phi(npos);
        real theta = local_angle::theta(npos);
        real u = math::clamp<real>(phi / (2 * PI_r), 0, 1);
        real v = math::clamp<real>(theta / PI_r, 0, 1);
        return tex_->sample_spectrum({ u, v });
    }
};

EnvirLight *create_hdri_light(
    const Texture *tex,
    const Vec3 &up,
    real radius,
    const Vec3 &offset,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena)
{
    auto ret = arena.create<HDRI>(customed_flag);
    ret->initialize(tex, up, radius, offset);
    return ret;
}

AGZT_IMPLEMENTATION(EnvirLight, HDRI, "hdri")

AGZ_TRACER_END
