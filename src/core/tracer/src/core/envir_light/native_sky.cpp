#include <agz/tracer/core/light.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

class NativeSky : public EnvirLight
{
    Spectrum top_;
    Spectrum bottom_;
    Vec3 up_ = Vec3(0, 0, 1);

public:

    void initialize(const Spectrum &top, const Spectrum &bottom, const Vec3 &up)
    {
        AGZ_HIERARCHY_TRY

        top_ = top;
        bottom_ = bottom;
        up_ = up.normalize();

        AGZ_HIERARCHY_WRAP("in initializing native sky")
    }

    LightSampleResult sample(const Vec3 &ref, const Sample5 &sam) const noexcept override
    {
        auto [dir, pdf] = math::distribution::uniform_on_sphere(sam.u, sam.v);

        LightSampleResult ret;
        ret.ref          = ref;
        ret.pos          = ref + 4 * world_radius_ * dir;
        ret.radiance     = radiance(ref, dir, nullptr);
        ret.pdf          = pdf;
        ret.is_delta     = false;

        return ret;
    }

    real pdf(const Vec3 &ref, const Vec3 &) const noexcept override
    {
        return math::distribution::uniform_on_sphere_pdf<real>;
    }

    Spectrum power() const noexcept override
    {
        real radius = world_radius_;
        Spectrum mean_radiance = (top_ + bottom_) * real(0.5);
        return 4 * PI_r * PI_r * radius * radius * mean_radiance;
    }

    Spectrum radiance(const Vec3 &ref, const Vec3 &ref_to_light, Vec3 *light_pnt) const noexcept override
    {
        if(light_pnt)
            *light_pnt = ref + 4 * world_radius_ * ref_to_light.normalize();
        real cos_theta = math::clamp<real>(dot(ref_to_light.normalize(), up_), -1, 1);
        real s = (cos_theta + 1) / 2;
        return s * top_ + (1 - s) * bottom_;
    }
};

std::shared_ptr<NonareaLight>create_native_sky(
    const Spectrum &top,
    const Spectrum &bottom,
    const Vec3 &up)
{
    auto ret = std::make_shared<NativeSky>();
    ret->initialize(top, bottom, up);
    return ret;
}

AGZ_TRACER_END
