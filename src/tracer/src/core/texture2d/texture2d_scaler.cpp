#include <agz/tracer/core/texture2d.h>

AGZ_TRACER_BEGIN

class Texture2DScaler : public Texture2D
{
    Spectrum scale_;
    std::shared_ptr<const Texture2D> internal_;

public:

    Texture2DScaler(
        const Texture2DCommonParams &common_params,
        const Spectrum &scale, std::shared_ptr<const Texture2D> internal)
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

std::shared_ptr<Texture2D> create_texture2d_scaler(
    const Texture2DCommonParams &common_params,
    const Spectrum &scale,
    std::shared_ptr<const Texture2D> internal)
{
    return std::make_shared<Texture2DScaler>(common_params, scale, internal);
}

AGZ_TRACER_END
