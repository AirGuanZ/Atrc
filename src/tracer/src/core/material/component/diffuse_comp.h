#pragma once

#include "./component.h"

AGZ_TRACER_BEGIN

class DiffuseComponent final : public BSDFComponent
{
    FSpectrum coef_; // albedo / pi

public:

    explicit DiffuseComponent(const FSpectrum &albedo) noexcept;

    FSpectrum eval(const FVec3 &lwi, const FVec3 &lwo, TransMode mode) const override;

    real pdf(const FVec3 &lwi, const FVec3 &lwo) const override;

    SampleResult sample(const FVec3 &lwo, TransMode mode, const Sample2 &sam) const override;

    BidirSampleResult sample_bidir(const FVec3 &lwo, TransMode mode, const Sample2 &sam) const override;

    bool has_diffuse_component() const override;
};

AGZ_TRACER_END
