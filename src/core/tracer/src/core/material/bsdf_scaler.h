#pragma once

#include <agz/tracer/core/bsdf.h>

AGZ_TRACER_BEGIN

class BSDFScaler : public BSDF
{
    Spectrum scale_;
    const BSDF *internal_;

public:

    BSDFScaler(const Spectrum &scale, const BSDF *internal) noexcept
        : scale_(scale), internal_(internal)
    {
        
    }

    Spectrum eval(const Vec3 &wi, const Vec3 &wo, TransportMode mode) const noexcept override
    {
        return scale_ * internal_->eval(wi, wo, mode);
    }

    BSDFSampleResult sample(const Vec3 &wo, TransportMode mode, const Sample3 &sam) const noexcept override
    {
        auto ret = internal_->sample(wo, mode, sam);
        ret.f *= scale_;
        return ret;
    }

    real pdf(const Vec3 &wi, const Vec3 &wo) const noexcept override
    {
        return internal_->pdf(wi, wo);
    }

    Spectrum albedo() const noexcept override
    {
        return scale_ * internal_->albedo();
    }

    bool is_delta() const noexcept override
    {
        return internal_->is_delta();
    }
};

AGZ_TRACER_END
