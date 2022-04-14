#pragma once

#include "component.h"

AGZ_TRACER_BEGIN

class GGXMicrofacetRefractionComponent final : public BSDFComponent
{
    FSpectrum color_;
    real ior_;
    real ax_;
    real ay_;

    FVec3 sample_trans(const FVec3 &lwo, const Sample2 &sam) const;

    FVec3 sample_inner_refl(const FVec3 &lwo, const Sample2 &sam) const;

    real pdf_trans(const FVec3 &lwi, const FVec3 &lwo) const;

    real pdf_inner_refl(const FVec3 &lwi, const FVec3 &lwo) const;

public:

    GGXMicrofacetRefractionComponent(const FSpectrum &color, real ior, real roughness, real anisotropic);

    FSpectrum eval(const FVec3 &lwi, const FVec3 &lwo, TransMode mode) const override;

    SampleResult sample(const FVec3 &lwo, TransMode mode, const Sample2 &sam) const override;

    BidirSampleResult sample_bidir(const FVec3 &lwo, TransMode mode, const Sample2 &sam) const override;

    real pdf(const FVec3 &lwi, const FVec3 &lwo) const override;

    bool has_diffuse_component() const override;
};

AGZ_TRACER_END
