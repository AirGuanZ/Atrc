#include <agz/tracer/core/texture2d.h>

AGZ_TRACER_BEGIN

class TextureAdder : public Texture2D
{
    std::shared_ptr<const Texture2D> lhs_;
    std::shared_ptr<const Texture2D> rhs_;

public:

    void initialize(
        const Texture2DCommonParams &common_params,
        std::shared_ptr<const Texture2D> lhs, std::shared_ptr<const Texture2D> rhs)
    {
        init_common_params(common_params);
        lhs_ = lhs;
        rhs_ = rhs;
    }

    Spectrum sample_spectrum(const Vec2 &uv) const noexcept override
    {
        Spectrum lhs_sample = lhs_->sample_spectrum(uv);
        Spectrum rhs_sample = rhs_->sample_spectrum(uv);
        return lhs_sample + rhs_sample;
    }

    int width() const noexcept override
    {
        return (std::max)(lhs_->width(), rhs_->width());
    }

    int height() const noexcept override
    {
        return (std::max)(lhs_->height(), rhs_->height());
    }
};

class TextureMultiplier : public Texture2D
{
    std::shared_ptr<const Texture2D> lhs_;
    std::shared_ptr<const Texture2D> rhs_;

public:

    void initialize(
        const Texture2DCommonParams &common_params,
        std::shared_ptr<const Texture2D> lhs, std::shared_ptr<const Texture2D> rhs)
    {
        init_common_params(common_params);
        lhs_ = lhs;
        rhs_ = rhs;
    }

    Spectrum sample_spectrum(const Vec2 &uv) const noexcept override
    {
        Spectrum lhs_sample = lhs_->sample_spectrum(uv);
        Spectrum rhs_sample = rhs_->sample_spectrum(uv);
        return lhs_sample * rhs_sample;
    }

    int width() const noexcept override
    {
        return (std::max)(lhs_->width(), rhs_->width());
    }

    int height() const noexcept override
    {
        return (std::max)(lhs_->height(), rhs_->height());
    }
};

std::shared_ptr<Texture2D> create_texture_adder(
    const Texture2DCommonParams &common_params,
    std::shared_ptr<const Texture2D> lhs,
    std::shared_ptr<const Texture2D> rhs)
{
    auto ret = std::make_shared<TextureAdder>();
    ret->initialize(common_params, std::move(lhs), std::move(rhs));
    return ret;
}

std::shared_ptr<Texture2D> create_texture_multiplier(
    const Texture2DCommonParams &common_params,
    std::shared_ptr<const Texture2D> lhs,
    std::shared_ptr<const Texture2D> rhs)
{
    auto ret = std::make_shared<TextureMultiplier>();
    ret->initialize(common_params, std::move(lhs), std::move(rhs));
    return ret;
}

AGZ_TRACER_END
