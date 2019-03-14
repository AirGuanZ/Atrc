#pragma once

#include <Atrc/Lib/Material/Utility/BxDF.h>
#include <Atrc/Lib/Material/Utility/Fresnel.h>

namespace Atrc
{

class BxDF_MicrofacetReflection : public BxDF
{
    Spectrum rc_;
    Real alpha_;
    const Fresnel *fresnel_;

public:

    BxDF_MicrofacetReflection(const Spectrum &rc, Real roughness, const Fresnel *fresnel) noexcept;

    Spectrum GetBaseColor() const noexcept override;

    Spectrum EvalUncolored(const Vec3 &wi, const Vec3 &wo, bool star) const noexcept override;

    std::optional<SampleWiResult> SampleWi(const Vec3 &wo, bool star, const Vec3 &sample) const noexcept override;

    Real SampleWiPDF(const Vec3 &wi, const Vec3 &wo, bool star) const noexcept override;
};

class BxDF_Microfacet : public BxDF
{
    Spectrum rc_;
    Real alpha_;
    const Dielectric *dielectric_;

public:

    BxDF_Microfacet(const Spectrum &rc, Real roughness, const Dielectric *dielectric) noexcept;

    Spectrum GetBaseColor() const noexcept override;

    Spectrum EvalUncolored(const Vec3 &wi, const Vec3 &wo, bool star) const noexcept override;

    std::optional<SampleWiResult> SampleWi(const Vec3 &wo, bool star, const Vec3 &sample) const noexcept override;

    Real SampleWiPDF(const Vec3 &wi, const Vec3 &wo, bool star) const noexcept override;
};

} // namespace Atrc
