#pragma once

#include "../utility/fresnel_point.h"
#include "./component.h"

AGZ_TRACER_BEGIN

class GGXMicrofacetReflectionComponent final : public BSDFComponent
{
    const FresnelPoint *fresnel_;
    real ax_;
    real ay_;

public:

    GGXMicrofacetReflectionComponent(const FresnelPoint *fresnel, real roughness, real anisotropic) noexcept;

    FSpectrum eval(const FVec3 &lwi, const FVec3 &lwo, TransMode mode) const override;

    SampleResult sample(const FVec3 &lwo, TransMode mode, const Sample2 &sam) const override;

    BidirSampleResult sample_bidir(const FVec3 &lwo, TransMode mode, const Sample2 &sam) const override;

    real pdf(const FVec3 &lwi, const FVec3 &lwo) const override;

    bool has_diffuse_component() const override;
};

AGZ_TRACER_END
