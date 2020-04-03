#pragma once

#include "component.h"

AGZ_TRACER_BEGIN

class GGXMicrofacetRefractionComponent : public BSDFComponent
{
    Spectrum color_;
    real ior_;
    real ax_;
    real ay_;

    Vec3 sample_trans(const Vec3 &lwo, const Sample2 &sam) const noexcept;

    Vec3 sample_inner_refl(const Vec3 &lwo, const Sample2 &sam) const noexcept;

    real pdf_trans(const Vec3 &lwi, const Vec3 &lwo) const noexcept;

    real pdf_inner_refl(const Vec3 &lwi, const Vec3 &lwo) const noexcept;

public:

    GGXMicrofacetRefractionComponent(
        const Spectrum &color, real ior,
        real roughness, real anisotropic);

    Spectrum eval(
        const Vec3 &lwi, const Vec3 &lwo,
        TransMode mode) const noexcept override;

    SampleResult sample(
        const Vec3 &lwo, TransMode mode,
        const Sample2 &sam) const noexcept override;

    real pdf(
        const Vec3 &lwi, const Vec3 &lwo) const noexcept override;
};

AGZ_TRACER_END
