#pragma once

#include "./component.h"

AGZ_TRACER_BEGIN

class PhongSpecularComponent : public BSDFComponent
{
    Spectrum s_;
    real ns_;

    FVec3 sample_pow_cos_on_hemisphere(
        real e, const Sample2 &sam) const noexcept;

    real pow_cos_on_hemisphere_pdf(real e, real cos_theta) const noexcept;

public:

    PhongSpecularComponent(const Spectrum &s, real ns) noexcept;

    Spectrum eval(
        const FVec3 &lwi, const FVec3 &lwo,
        TransMode mode) const noexcept override;

    SampleResult sample(
        const FVec3 &lwo, TransMode mode,
        const Sample2 &sam) const noexcept override;

    real pdf(const FVec3 &lwi, const FVec3 &lwo) const noexcept override;
};

AGZ_TRACER_END
