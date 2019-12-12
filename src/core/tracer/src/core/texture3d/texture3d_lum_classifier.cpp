#include <agz/tracer/core/texture3d.h>

AGZ_TRACER_BEGIN

class Texture3DLuminanceClassifier : public Texture3D
{
    std::shared_ptr<const Texture3D> lhs_;
    std::shared_ptr<const Texture3D> rhs_;
    std::shared_ptr<const Texture3D> less_or_equal_;
    std::shared_ptr<const Texture3D> greater_;

protected:

    Spectrum sample_spectrum_impl(const Vec3 &uvw) const noexcept override
    {
        Spectrum lhs = lhs_->sample_spectrum(uvw);
        Spectrum rhs = rhs_->sample_spectrum(uvw);
        if(lhs.lum() <= rhs.lum())
            return less_or_equal_->sample_spectrum(uvw);
        return greater_->sample_spectrum(uvw);
    }

    real sample_real_impl(const Vec3 &uvw) const noexcept override
    {
        real lhs = lhs_->sample_real(uvw);
        real rhs = rhs_->sample_real(uvw);
        if(lhs <= rhs)
            return less_or_equal_->sample_real(uvw);
        return greater_->sample_real(uvw);
    }

public:

    Texture3DLuminanceClassifier(
        const Texture3DCommonParams &common_params,
        std::shared_ptr<const Texture3D> lhs,
        std::shared_ptr<const Texture3D> rhs,
        std::shared_ptr<const Texture3D> less_or_equal,
        std::shared_ptr<const Texture3D> greater)
    {
        init_common_params(common_params);
        lhs_.swap(lhs);
        rhs_.swap(rhs);
        less_or_equal_.swap(less_or_equal);
        greater_.swap(greater);
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
        Spectrum less_or_equal = less_or_equal_->max_spectrum(), greater = greater_->max_spectrum();
        for(int c = 0; c < SPECTRUM_COMPONENT_COUNT; ++c)
            ret[c] = (std::max)(less_or_equal[c], greater[c]);
        return ret;
    }

    Spectrum min_spectrum() const noexcept override
    {
        Spectrum ret;
        Spectrum less_or_equal = less_or_equal_->min_spectrum(), greater = greater_->min_spectrum();
        for(int c = 0; c < SPECTRUM_COMPONENT_COUNT; ++c)
            ret[c] = (std::min)(less_or_equal[c], greater[c]);
        return ret;
    }

    real max_real() const noexcept override
    {
        return (std::max)(less_or_equal_->max_real(), greater_->max_real());
    }

    real min_real() const noexcept override
    {
        return (std::min)(less_or_equal_->min_real(), greater_->min_real());
    }
};

std::shared_ptr<Texture3D> create_texture3d_lum_classifier(
    const Texture3DCommonParams &common_params,
    std::shared_ptr<const Texture3D> lhs,
    std::shared_ptr<const Texture3D> rhs,
    std::shared_ptr<const Texture3D> less_or_equal,
    std::shared_ptr<const Texture3D> greater)
{
    return std::make_shared<Texture3DLuminanceClassifier>(
        common_params, std::move(lhs), std::move(rhs),
        std::move(less_or_equal), std::move(greater));
}

AGZ_TRACER_END
