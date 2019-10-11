#include <agz/tracer/core/texture.h>

AGZ_TRACER_BEGIN

class TextureScaler : public Texture
{
    Spectrum scale_;
    std::shared_ptr<const Texture> internal_;

public:

    void initialize(
        const TextureCommonParams &common_params,
        const Spectrum &scale, std::shared_ptr<const Texture> internal)
    {
        init_common_params(common_params);
        scale_ = scale;
        internal_ = internal;
    }

    Spectrum sample_spectrum(const Vec2 &uv) const noexcept override
    {
        return scale_ * internal_->sample_spectrum(uv);
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

std::shared_ptr<Texture> create_texture_scaler(
    const TextureCommonParams &common_params,
    const Spectrum &scale,
    std::shared_ptr<const Texture> internal)
{
    auto ret = std::make_shared<TextureScaler>();
    ret->initialize(common_params, scale, internal);
    return ret;
}

AGZ_TRACER_END
