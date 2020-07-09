#pragma once

#include "component.h"

AGZ_TRACER_BEGIN

class GGXMicrofacetRefractionComponent : public BSDFComponent
{
    FSpectrum color_;
    real ior_;
    real ax_;
    real ay_;

    FVec3 sample_trans(const FVec3 &lwo, const Sample2 &sam) const noexcept;

    FVec3 sample_inner_refl(const FVec3 &lwo, const Sample2 &sam) const noexcept;

    real pdf_trans(const FVec3 &lwi, const FVec3 &lwo) const noexcept;

    real pdf_inner_refl(const FVec3 &lwi, const FVec3 &lwo) const noexcept;

public:

    GGXMicrofacetRefractionComponent(
        const FSpectrum &color, real ior,
        real roughness, real anisotropic);

    FSpectrum eval(
        const FVec3 &lwi, const FVec3 &lwo,
        TransMode mode) const noexcept override;

    SampleResult sample(
        const FVec3 &lwo, TransMode mode,
        const Sample2 &sam) const noexcept override;

    real pdf(
        const FVec3 &lwi, const FVec3 &lwo) const noexcept override;
};

AGZ_TRACER_END
