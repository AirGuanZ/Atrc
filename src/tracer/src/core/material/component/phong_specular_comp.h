#pragma once

#include "./component.h"

AGZ_TRACER_BEGIN

class PhongSpecularComponent final : public BSDFComponent
{
    FSpectrum s_;
    real ns_;

    FVec3 sample_pow_cos_on_hemisphere(real e, const Sample2 &sam) const;

    real pow_cos_on_hemisphere_pdf(real e, real cos_theta) const;

public:

    PhongSpecularComponent(const FSpectrum &s, real ns) noexcept;

    FSpectrum eval(const FVec3 &lwi, const FVec3 &lwo, TransMode mode) const override;

    SampleResult sample(const FVec3 &lwo, TransMode mode, const Sample2 &sam) const override;

    BidirSampleResult sample_bidir(const FVec3 &lwo, TransMode mode, const Sample2 &sam) const override;

    real pdf(const FVec3 &lwi, const FVec3 &lwo) const override;

    bool has_diffuse_component() const override;
};

AGZ_TRACER_END
