#include <agz/tracer/core/texture.h>

AGZ_TRACER_BEGIN

class Constant : public Texture
{
    Spectrum texel_;

protected:

    Spectrum sample_spectrum_impl(const Vec2&) const noexcept override
    {
        return texel_;
    }

public:

    void initialize(const TextureCommonParams &common_params, const Spectrum &texel)
    {
        init_common_params(common_params);
        texel_ = texel;
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

std::shared_ptr<Texture> create_constant_texture(
    const TextureCommonParams &common_params,
    const Spectrum &texel)
{
    auto ret = std::make_shared<Constant>();
    ret->initialize(common_params, texel);
    return ret;
}

std::shared_ptr<Texture> create_constant_texture(
    const TextureCommonParams &common_params,
    real texel)
{
    return create_constant_texture(common_params, Spectrum(texel));
}

AGZ_TRACER_END
