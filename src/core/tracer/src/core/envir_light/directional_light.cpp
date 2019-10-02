#include <agz/tracer/core/light.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

class DirectionalEnvironmentLight : public EnvirLight
{
    // local light cone总是从+z方向向-z方向照射
    Coord local_cone_space_;
    Spectrum radiance_;
    real max_cos_theta_ = real(0.5);

    static Vec2 dir_to_uv(const Vec3 &d)
    {
        Vec3 dir = d.normalize();
        real phi = local_angle::phi(dir);
        real theta = local_angle::theta(dir);
        real u = math::clamp<real>(phi / (2 * PI_r), 0, 1);
        real v = math::clamp<real>(theta / PI_r, 0, 1);
        return { u, v };
    }

public:

    void initialize(const Vec3 &dir, const Spectrum &radiance, real range)
    {
        AGZ_HIERARCHY_TRY

        local_cone_space_ = Coord::from_z(-dir);
        radiance_ = radiance;

        max_cos_theta_ = 1 - range;
        if(max_cos_theta_ <= 0 || max_cos_theta_ >= 1)
            throw ObjectConstructionException("invalid range value");

        AGZ_HIERARCHY_WRAP("in initializing directional environment light")
    }

    EnvirLightSampleResult sample(const Vec3 &ref, const Sample3 &sam) const noexcept override
    {
        auto [local_dir, pdf] = math::distribution::uniform_on_cone(max_cos_theta_, sam.u, sam.v);
        Vec3 global_dir = local_cone_space_.local_to_global(local_dir).normalize();

        EnvirLightSampleResult ret;
        ret.ref_to_light = global_dir;
        ret.radiance     = radiance(ref, global_dir);
        ret.pdf          = pdf;
        ret.is_delta     = false;

        return ret;
    }

    real pdf(const Vec3 &ref, const Vec3 &ref_to_light) const noexcept override
    {
        Vec3 local_dir = local_cone_space_.global_to_local(ref_to_light);
        real cos_theta = local_angle::cos_theta(local_dir.normalize());
        if(cos_theta >= max_cos_theta_)
            return math::distribution::uniform_on_cone_pdf(max_cos_theta_);
        return 0;
    }

    Spectrum power() const noexcept override
    {
        real radius = world_radius_;
        return 4 * PI_r * PI_r * radius * radius * ((1 - max_cos_theta_) / 2 * radiance_);
    }

    Spectrum radiance(const Vec3 &ref, const Vec3 &ref_to_light) const noexcept override
    {
        Vec3 local_dir = local_cone_space_.global_to_local(ref_to_light);
        real cos_theta = local_angle::cos_theta(local_dir.normalize());
        if(cos_theta >= max_cos_theta_)
            return radiance_;
        return {};
    }
};

std::shared_ptr<EnvirLight> create_directional_light(
    const Vec3 &dir,
    const Spectrum &radiance,
    real range)
{
    auto ret = std::make_shared<DirectionalEnvironmentLight>();
    ret->initialize(dir, radiance, range);
    return ret;
}

AGZ_TRACER_END
