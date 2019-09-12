#include <agz/tracer/core/light.h>

AGZ_TRACER_BEGIN

class NativeSky : public EnvirLight
{
    Spectrum top_;
    Spectrum bottom_;
    Vec3 up_ = Vec3(0, 0, 1);

public:

    static std::string description()
    {
        return R"___(
native_sky [EnvironmentLight]
    top    [Spectrum] top radiance
    bottom [Spectrum] bottom radiance
    up     [Vec3]     up direction
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        init_customed_flag(params);

        top_ = params.child_spectrum("top");
        bottom_ = params.child_spectrum("bottom");

        if(params.find_child("up"))
            up_ = params.child_vec3("up").normalize();
        else
            up_ = Vec3(0, 0, 1);

        AGZ_HIERARCHY_WRAP("in initializing native sky light")
    }

    EnvirLightSampleResult sample(const Vec3 &ref, const Sample3 &sam) const noexcept override
    {
        auto [dir, pdf] = math::distribution::uniform_on_sphere(sam.u, sam.v);

        EnvirLightSampleResult ret;
        ret.ref_to_light = dir;
        ret.radiance     = radiance(ref, dir);
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

    Spectrum radiance(const Vec3 &ref, const Vec3 &ref_to_light) const noexcept override
    {
        real cos_theta = math::clamp<real>(dot(ref_to_light.normalize(), up_), -1, 1);
        real s = (cos_theta + 1) / 2;
        return s * top_ + (1 - s) * bottom_;
    }
};

AGZT_IMPLEMENTATION(EnvirLight, NativeSky, "native_sky")

AGZ_TRACER_END
