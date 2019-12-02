#include <agz/tracer/core/texture2d.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

class GradientTexture : public Texture2D
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

    void initialize(const Texture2DCommonParams &common_params, const Spectrum &color1, const Spectrum &color2)
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

std::shared_ptr<Texture2D> create_gradient_texture(
    const Texture2DCommonParams &common_params,
    const Spectrum &color1, const Spectrum &color2)
{
    auto ret = std::make_shared<GradientTexture>();
    ret->initialize(common_params, color1, color2);
    return ret;
}

AGZ_TRACER_END
