#pragma once

#include "./component.h"

AGZ_TRACER_BEGIN

class DiffuseComponent : public BSDFComponent
{
    FSpectrum coef_; // albedo / pi

public:

    explicit DiffuseComponent(const FSpectrum &albedo) noexcept;

    FSpectrum eval(
        const FVec3 &lwi, const FVec3 &lwo,
        TransMode mode) const noexcept override;

    real pdf(const FVec3 &lwi, const FVec3 &lwo) const noexcept override;

    SampleResult sample(
        const FVec3 &lwo, TransMode mode,
        const Sample2 &sam) const noexcept override;
};

AGZ_TRACER_END
