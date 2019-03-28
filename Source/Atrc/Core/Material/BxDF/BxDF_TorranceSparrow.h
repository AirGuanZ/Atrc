#pragma once

#include <Atrc/Core/Material/Utility/BxDF.h>
#include <Atrc/Core/Material/Utility/Fresnel.h>
#include <Atrc/Core/Material/Utility/MicrofacetDistribution.h>

namespace Atrc
{

class BxDF_TorranceSparrowReflection : public BxDF
{
    Spectrum rc_;
    const MicrofacetDistribution *md_;
    const Fresnel *fresnel_;

public:

    BxDF_TorranceSparrowReflection(const Spectrum &rc, const MicrofacetDistribution *md, const Fresnel *fresnel) noexcept;

    Spectrum GetBaseColor() const noexcept override;

    Spectrum EvalUncolored(const Vec3 &wi, const Vec3 &wo, bool star) const noexcept override;

    std::optional<SampleWiResult> SampleWi(const Vec3 &wo, bool star, const Vec3 &sample) const noexcept override;

    Real SampleWiPDF(const Vec3 &wi, const Vec3 &wo, bool star) const noexcept override;
};

} // namespace Atrc
