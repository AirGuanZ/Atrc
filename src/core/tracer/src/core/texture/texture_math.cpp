#include <agz/tracer/core/texture.h>

AGZ_TRACER_BEGIN

class TextureAdder : public Texture
{
    std::shared_ptr<const Texture> lhs_;
    std::shared_ptr<const Texture> rhs_;

public:

    void initialize(
        const TextureCommonParams &common_params,
        std::shared_ptr<const Texture> lhs, std::shared_ptr<const Texture> rhs)
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

class TextureMultiplier : public Texture
{
    std::shared_ptr<const Texture> lhs_;
    std::shared_ptr<const Texture> rhs_;

public:

    void initialize(
        const TextureCommonParams &common_params,
        std::shared_ptr<const Texture> lhs, std::shared_ptr<const Texture> rhs)
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

std::shared_ptr<Texture> create_texture_adder(
    const TextureCommonParams &common_params,
    std::shared_ptr<const Texture> lhs,
    std::shared_ptr<const Texture> rhs)
{
    auto ret = std::make_shared<TextureAdder>();
    ret->initialize(common_params, std::move(lhs), std::move(rhs));
    return ret;
}

std::shared_ptr<Texture> create_texture_multiplier(
    const TextureCommonParams &common_params,
    std::shared_ptr<const Texture> lhs,
    std::shared_ptr<const Texture> rhs)
{
    auto ret = std::make_shared<TextureMultiplier>();
    ret->initialize(common_params, std::move(lhs), std::move(rhs));
    return ret;
}

AGZ_TRACER_END
