#include <agz/tracer/core/texture.h>

AGZ_TRACER_BEGIN

class TextureReverse : public Texture
{
    std::shared_ptr<const Texture> internal_;

    Spectrum sample_spectrum_impl(const Vec2 &uv) const noexcept override
    {
        auto origin = internal_->sample_spectrum(uv);
        return (Spectrum(1) - origin).saturate();
    }

    real sample_real_impl(const Vec2 &uv) const noexcept override
    {
        auto origin = internal_->sample_real(uv);
        return agz::math::saturate<real>(1 - origin);
    }

public:

    TextureReverse(
        const TextureCommonParams &common_params,
        std::shared_ptr<const Texture> internal)
    {
        init_common_params(common_params);
        internal_ = std::move(internal);
    }

    int width() const noexcept override
    {
        return internal_->width();
    }

    int height() const noexcept override
    {
        return internal_->height();
    }
};

std::shared_ptr<Texture> create_texture_reverse(
    const TextureCommonParams &common_params,
    std::shared_ptr<const Texture> internal)
{
    return std::make_shared<TextureReverse>(common_params, std::move(internal));
}

AGZ_TRACER_END
