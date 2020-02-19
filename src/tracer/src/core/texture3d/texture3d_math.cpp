#include <agz/tracer/core/texture3d.h>

AGZ_TRACER_BEGIN

class Texture3DAdder : public Texture3D
{
    std::shared_ptr<const Texture3D> lhs_;
    std::shared_ptr<const Texture3D> rhs_;

protected:

    Spectrum sample_spectrum_impl(const Vec3 &uvw) const noexcept override
    {
        const Spectrum lhs_spectrum = lhs_->sample_spectrum(uvw);
        const Spectrum rhs_spectrum = rhs_->sample_spectrum(uvw);
        return lhs_spectrum + rhs_spectrum;
    }

    real sample_real_impl(const Vec3 &uvw) const noexcept override
    {
        const real lhs_real = lhs_->sample_real(uvw);
        const real rhs_real = rhs_->sample_real(uvw);
        return lhs_real + rhs_real;
    }

public:

    Texture3DAdder(
        const Texture3DCommonParams &common_params,
        std::shared_ptr<const Texture3D> lhs,
        std::shared_ptr<const Texture3D> rhs)
    {
        init_common_params(common_params);
        lhs_.swap(lhs);
        rhs_.swap(rhs);
    }

    int width() const noexcept override
    {
        return lhs_->width();
    }

    int height() const noexcept override
    {
        return lhs_->height();
    }

    int depth() const noexcept override
    {
        return lhs_->depth();
    }

    Spectrum max_spectrum() const noexcept override
    {
        return lhs_->max_spectrum() + rhs_->max_spectrum();
    }

    Spectrum min_spectrum() const noexcept override
    {
        return lhs_->min_spectrum() + rhs_->min_spectrum();
    }

    real max_real() const noexcept override
    {
        return lhs_->max_real() + rhs_->max_real();
    }

    real min_real() const noexcept override
    {
        return lhs_->min_real() + rhs_->min_real();
    }
};

class Texture3DMultiplier : public Texture3D
{
    std::shared_ptr<const Texture3D> lhs_;
    std::shared_ptr<const Texture3D> rhs_;

protected:

    Spectrum sample_spectrum_impl(const Vec3 &uvw) const noexcept override
    {
        const Spectrum lhs_spectrum = lhs_->sample_spectrum(uvw);
        const Spectrum rhs_spectrum = rhs_->sample_spectrum(uvw);
        return lhs_spectrum * rhs_spectrum;
    }

    real sample_real_impl(const Vec3 &uvw) const noexcept override
    {
        const real lhs_real = lhs_->sample_real(uvw);
        const real rhs_real = rhs_->sample_real(uvw);
        return lhs_real * rhs_real;
    }

public:

    Texture3DMultiplier(
        const Texture3DCommonParams &common_params,
        std::shared_ptr<const Texture3D> lhs,
        std::shared_ptr<const Texture3D> rhs)
    {
        init_common_params(common_params);
        lhs_.swap(lhs);
        rhs_.swap(rhs);
    }

    int width() const noexcept override
    {
        return lhs_->width();
    }

    int height() const noexcept override
    {
        return lhs_->height();
    }

    int depth() const noexcept override
    {
        return lhs_->depth();
    }

    Spectrum max_spectrum() const noexcept override
    {
        Spectrum ret;
        const Spectrum a0 = lhs_->min_spectrum(), a1 = lhs_->max_spectrum();
        const Spectrum b0 = rhs_->min_spectrum(), b1 = rhs_->max_spectrum();
        for(int c = 0; c < SPECTRUM_COMPONENT_COUNT; ++c)
            ret[c] = (std::max)({ a0[c] * b0[c], a0[c] * b1[c], a1[c] * b0[c], a1[c] * b1[c] });
        return ret;
    }

    Spectrum min_spectrum() const noexcept override
    {
        Spectrum ret;
        const Spectrum a0 = lhs_->min_spectrum(), a1 = lhs_->max_spectrum();
        const Spectrum b0 = rhs_->min_spectrum(), b1 = rhs_->max_spectrum();
        for(int c = 0; c < SPECTRUM_COMPONENT_COUNT; ++c)
            ret[c] = (std::min)({ a0[c] * b0[c], a0[c] * b1[c], a1[c] * b0[c], a1[c] * b1[c] });
        return ret;
    }

    real max_real() const noexcept override
    {
        const real a0 = lhs_->min_real(), a1 = rhs_->max_real();
        const real b0 = rhs_->min_real(), b1 = rhs_->max_real();
        return (std::max)({ a0 * b0, a0 * b1, a1 * b0, a1 * b1 });
    }

    real min_real() const noexcept override
    {
        const real a0 = lhs_->min_real(), a1 = rhs_->max_real();
        const real b0 = rhs_->min_real(), b1 = rhs_->max_real();
        return (std::min)({ a0 * b0, a0 * b1, a1 * b0, a1 * b1 });
    }
};

class Texture3DScaler : public Texture3D
{
    std::shared_ptr<const Texture3D> internal_;
    Spectrum scale_;

protected:

    Spectrum sample_spectrum_impl(const Vec3 &uvw) const noexcept override
    {
        return scale_ * internal_->sample_spectrum(uvw);
    }

    real sample_real_impl(const Vec3 &uvw) const noexcept override
    {
        return scale_.r * internal_->sample_real(uvw);
    }

public:

    Texture3DScaler(
        const Texture3DCommonParams &common_params,
        std::shared_ptr<const Texture3D> internal,
        const Spectrum &scale)
    {
        init_common_params(common_params);
        internal_.swap(internal);
        scale_ = scale;
    }

    int width() const noexcept override
    {
        return internal_->width();
    }

    int height() const noexcept override
    {
        return internal_->height();
    }

    int depth() const noexcept override
    {
        return internal_->depth();
    }

    Spectrum max_spectrum() const noexcept override
    {
        Spectrum ret;
        const Spectrum i0 = internal_->min_spectrum(), i1 = internal_->max_spectrum();
        for(int c = 0; c < SPECTRUM_COMPONENT_COUNT; ++c)
            ret[c] = (std::max)(scale_[c] * i0[c], scale_[c] * i1[c]);
        return ret;
    }

    Spectrum min_spectrum() const noexcept override
    {
        Spectrum ret;
        const Spectrum i0 = internal_->min_spectrum(), i1 = internal_->max_spectrum();
        for(int c = 0; c < SPECTRUM_COMPONENT_COUNT; ++c)
            ret[c] = (std::min)(scale_[c] * i0[c], scale_[c] * i1[c]);
        return ret;
    }

    real max_real() const noexcept override
    {
        return (std::max)(scale_.r * internal_->min_real(), scale_.r * internal_->max_real());
    }

    real min_real() const noexcept override
    {
        return (std::min)(scale_.r * internal_->min_real(), scale_.r * internal_->max_real());
    }
};

std::shared_ptr<Texture3D> create_texture3d_adder(
    const Texture3DCommonParams &common_params,
    std::shared_ptr<const Texture3D> lhs,
    std::shared_ptr<const Texture3D> rhs)
{
    return std::make_shared<Texture3DAdder>(
        common_params, std::move(lhs), std::move(rhs));
}

std::shared_ptr<Texture3D> create_texture3d_multiplier(
    const Texture3DCommonParams &common_params,
    std::shared_ptr<const Texture3D> lhs,
    std::shared_ptr<const Texture3D> rhs)
{
    return std::make_shared<Texture3DMultiplier>(
        common_params, std::move(lhs), std::move(rhs));
}

std::shared_ptr<Texture3D> create_texture3d_scaler(
    const Texture3DCommonParams &common_params,
    std::shared_ptr<const Texture3D> internal,
    const Spectrum &scale)
{
    return std::make_shared<Texture3DScaler>(
        common_params, std::move(internal), scale);
}

AGZ_TRACER_END
