#include <agz/tracer/core/texture.h>

AGZ_TRACER_BEGIN

class GradientTexture : public Texture
{
    Spectrum color1_;
    Spectrum color2_;

protected:

    Spectrum sample_spectrum_impl(const Vec2 &uv) const noexcept override
    {
        real t = math::saturate(uv.x);
        return mix(color1_, color2_, t);
    }

public:

    explicit GradientTexture(CustomedFlag customed_flag = DEFAULT_CUSTOMED_FLAG)
    {
        object_customed_flag_ = customed_flag;
    }

    static std::string description()
    {
        return R"___(
gradient [Texture]
    color1 [Spectrum] first color
    color2 [Spectrum] second color

    linear gradient texture
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext&) override
    {
        AGZ_HIERARCHY_TRY

        color1_ = params.child_spectrum("color1");
        color2_ = params.child_spectrum("color2");

        AGZ_HIERARCHY_WRAP("in initializing gradient texture")
    }

    void initialize(const TextureCommonParams &common_params, const Spectrum &color1, const Spectrum &color2)
    {
        AGZ_HIERARCHY_TRY

        init_common_params(common_params);

        color1_ = color1;
        color2_ = color2;

        AGZ_HIERARCHY_WRAP("in initializing gradient texture")
    }

    int width() const noexcept override
    {
        return 1;
    }

    int height() const noexcept override
    {
        return 1;
    }
};

Texture *create_gradient_texture(
    const TextureCommonParams &common_params,
    const Spectrum &color1, const Spectrum &color2,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena)
{
    auto ret = arena.create<GradientTexture>(customed_flag);
    ret->initialize(common_params, color1, color2);
    return ret;
}

AGZT_IMPLEMENTATION(Texture, GradientTexture, "gradient")

AGZ_TRACER_END
