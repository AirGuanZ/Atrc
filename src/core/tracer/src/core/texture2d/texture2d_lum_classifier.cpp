#include <agz/tracer/core/texture2d.h>

AGZ_TRACER_BEGIN

class Texture2DLuminanceClassifier : public Texture2D
{
    std::shared_ptr<const Texture2D> internal_;
    std::shared_ptr<const Texture2D> threshold_;
    std::shared_ptr<const Texture2D> higher_;
    std::shared_ptr<const Texture2D> lower_;

protected:

    Spectrum sample_spectrum_impl(const Vec2 &uv) const noexcept override
    {
        const Spectrum origin = internal_->sample_spectrum(uv);
        const real threshold = threshold_->sample_real(uv);
        if(origin.lum() <= threshold)
            return lower_->sample_spectrum(uv);
        return higher_->sample_spectrum(uv);
    }

public:

    Texture2DLuminanceClassifier(
        const Texture2DCommonParams &common_params,
        std::shared_ptr<const Texture2D> internal,
        std::shared_ptr<const Texture2D> threshold,
        std::shared_ptr<const Texture2D> higher,
        std::shared_ptr<const Texture2D> lower)
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

std::shared_ptr<Texture2D> create_texture2d_luminance_classifier(
    const Texture2DCommonParams &common_params,
    std::shared_ptr<const Texture2D> internal,
    std::shared_ptr<const Texture2D> threshold_texture,
    std::shared_ptr<const Texture2D> higher_texture,
    std::shared_ptr<const Texture2D> lower_texture)
{
    return std::make_shared<Texture2DLuminanceClassifier>(
                    common_params,
                    std::move(internal),
                    std::move(threshold_texture),
                    std::move(higher_texture),
                    std::move(lower_texture));
}

AGZ_TRACER_END
