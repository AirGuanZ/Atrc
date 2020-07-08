#pragma once

#include "./component.h"

AGZ_TRACER_BEGIN

class DiffuseComponent : public BSDFComponent
{
    Spectrum coef_; // albedo / pi

public:

    explicit DiffuseComponent(const Spectrum &albedo) noexcept;

    Spectrum eval(
        const FVec3 &lwi, const FVec3 &lwo,
        TransMode mode) const noexcept override;

    real pdf(const FVec3 &lwi, const FVec3 &lwo) const noexcept override;

    SampleResult sample(
        const FVec3 &lwo, TransMode mode,
        const Sample2 &sam) const noexcept override;
};

AGZ_TRACER_END
