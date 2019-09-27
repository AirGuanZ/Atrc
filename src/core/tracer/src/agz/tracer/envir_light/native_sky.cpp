#include <agz/tracer/core/light.h>

AGZ_TRACER_BEGIN

class NativeSky : public EnvirLight
{
    Spectrum top_;
    Spectrum bottom_;
    Vec3 up_ = Vec3(0, 0, 1);

public:

    explicit NativeSky(CustomedFlag customed_flag = DEFAULT_CUSTOMED_FLAG)
    {
        object_customed_flag_ = customed_flag;
    }

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

        Spectrum top = params.child_spectrum("top");
        Spectrum bottom = params.child_spectrum("bottom");
        
        Vec3 up(0, 0, 1);
        if(params.find_child("up"))
            up = params.child_vec3("up").normalize();

        initialize(top, bottom, up);

        AGZ_HIERARCHY_WRAP("in initializing native sky light")
    }

    void initialize(const Spectrum &top, const Spectrum &bottom, const Vec3 &up)
    {
        AGZ_HIERARCHY_TRY

        top_ = top;
        bottom_ = bottom;
        up_ = up.normalize();

        AGZ_HIERARCHY_WRAP("in initializing native sky")
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

EnvirLight *create_native_sky(
    const Spectrum &top,
    const Spectrum &bottom,
    const Vec3 &up,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena)
{
    auto ret = arena.create<NativeSky>(customed_flag);
    ret->initialize(top, bottom, up);
    return ret;
}

AGZT_IMPLEMENTATION(EnvirLight, NativeSky, "native_sky")

AGZ_TRACER_END
