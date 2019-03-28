#pragma once

#include <Atrc/Core/Material/Utility/BxDF.h>
#include <Atrc/Core/Material/Utility/Fresnel.h>

namespace Atrc
{
    
class BxDF_SpecularReflection : public BxDF
{
    const Fresnel *fresnel_;
    Spectrum rc_;

public:

    BxDF_SpecularReflection(const Fresnel *fresnel, const Spectrum &rc) noexcept;

    Spectrum GetBaseColor() const noexcept override;

    Spectrum EvalUncolored(const Vec3 &wi, const Vec3 &wo, bool star) const noexcept override;

    std::optional<SampleWiResult> SampleWi(const Vec3 &wo, bool star, const Vec3 &sample) const noexcept override;

    Real SampleWiPDF(const Vec3 &wi, const Vec3 &wo, bool star) const noexcept override;
};

} // namespace Atrc
