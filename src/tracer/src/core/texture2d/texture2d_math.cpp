#include <agz/tracer/core/texture2d.h>

AGZ_TRACER_BEGIN

class Texture2DAdder : public Texture2D
{
    std::shared_ptr<const Texture2D> lhs_;
    std::shared_ptr<const Texture2D> rhs_;

public:

    Texture2DAdder(
        const Texture2DCommonParams &common_params,
        std::shared_ptr<const Texture2D> lhs, std::shared_ptr<const Texture2D> rhs)
    {
        init_common_params(common_params);
        lhs_ = lhs;
        rhs_ = rhs;
    }

    Spectrum sample_spectrum(const Vec2 &uv) const noexcept override
    {
        const Spectrum lhs_sample = lhs_->sample_spectrum(uv);
        const Spectrum rhs_sample = rhs_->sample_spectrum(uv);
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

class Texture2DMultiplier : public Texture2D
{
    std::shared_ptr<const Texture2D> lhs_;
    std::shared_ptr<const Texture2D> rhs_;

public:

    Texture2DMultiplier(
        const Texture2DCommonParams &common_params,
        std::shared_ptr<const Texture2D> lhs, std::shared_ptr<const Texture2D> rhs)
    {
        init_common_params(common_params);
        lhs_ = lhs;
        rhs_ = rhs;
    }

    Spectrum sample_spectrum(const Vec2 &uv) const noexcept override
    {
        const Spectrum lhs_sample = lhs_->sample_spectrum(uv);
        const Spectrum rhs_sample = rhs_->sample_spectrum(uv);
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

std::shared_ptr<Texture2D> create_texture2d_adder(
    const Texture2DCommonParams &common_params,
    std::shared_ptr<const Texture2D> lhs,
    std::shared_ptr<const Texture2D> rhs)
{
    return std::make_shared<Texture2DAdder>(common_params, std::move(lhs), std::move(rhs));
}

std::shared_ptr<Texture2D> create_texture2d_multiplier(
    const Texture2DCommonParams &common_params,
    std::shared_ptr<const Texture2D> lhs,
    std::shared_ptr<const Texture2D> rhs)
{
    return std::make_shared<Texture2DMultiplier>(common_params, std::move(lhs), std::move(rhs));
}

AGZ_TRACER_END
