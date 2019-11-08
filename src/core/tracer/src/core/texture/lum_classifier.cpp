#include <agz/tracer/core/texture.h>

AGZ_TRACER_BEGIN

class LuminanceClassifier : public Texture
{
    std::shared_ptr<const Texture> internal_;
    std::shared_ptr<const Texture> threshold_;
    std::shared_ptr<const Texture> higher_;
    std::shared_ptr<const Texture> lower_;

protected:

    Spectrum sample_spectrum_impl(const Vec2 &uv) const noexcept override
    {
        Spectrum origin = internal_->sample_spectrum(uv);
        real threshold = threshold_->sample_real(uv);
        if(origin.lum() <= threshold)
            return lower_->sample_spectrum(uv);
        return higher_->sample_spectrum(uv);
    }

public:

    LuminanceClassifier(
        const TextureCommonParams &common_params,
        std::shared_ptr<const Texture> internal,
        std::shared_ptr<const Texture> threshold,
        std::shared_ptr<const Texture> higher,
        std::shared_ptr<const Texture> lower)
    {
        init_common_params(common_params);
        internal_  = std::move(internal);
        threshold_ = std::move(threshold);
        higher_    = std::move(higher);
        lower_     = std::move(lower);
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

std::shared_ptr<Texture> create_luminance_classifier(
    const TextureCommonParams &common_params,
    std::shared_ptr<const Texture> internal,
    std::shared_ptr<const Texture> threshold_texture,
    std::shared_ptr<const Texture> higher_texture,
    std::shared_ptr<const Texture> lower_texture)
{
    return std::make_shared<LuminanceClassifier>(
                    common_params,
                    std::move(internal),
                    std::move(threshold_texture),
                    std::move(higher_texture),
                    std::move(lower_texture));
}

AGZ_TRACER_END
