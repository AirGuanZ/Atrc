#include <agz/tracer/core/texture2d.h>

AGZ_TRACER_BEGIN

class Constant2D : public Texture2D
{
    Spectrum texel_;

public:

    Constant2D(
        const Texture2DCommonParams &common_params, const Spectrum &texel)
    {
        init_common_params(common_params);
        texel_ = texel;

        if(inv_gamma_ != 1)
        {
            for(int i = 0; i < SPECTRUM_COMPONENT_COUNT; ++i)
                texel_[i] = std::pow(texel_[i], inv_gamma_);
        }
    }

    int width() const noexcept override
    {
        return 1;
    }

    int height() const noexcept override
    {
        return 1;
    }

    Spectrum sample_spectrum(const Vec2 &uv) const noexcept override
    {
        return texel_;
    }

    real sample_real(const Vec2 &uv) const noexcept override
    {
        return texel_.r;
    }
};

RC<Texture2D> create_constant2d_texture(
    const Texture2DCommonParams &common_params,
    const Spectrum &texel)
{
    return newRC<Constant2D>(common_params, texel);
}

RC<Texture2D> create_constant2d_texture(
    const Texture2DCommonParams &common_params,
    real texel)
{
    return create_constant2d_texture(common_params, Spectrum(texel));
}

AGZ_TRACER_END
