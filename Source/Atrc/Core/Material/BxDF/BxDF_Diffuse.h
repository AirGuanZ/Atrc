#pragma once

#include <Atrc/Core/Material/Utility/BxDF.h>

namespace Atrc
{

class BxDF_Diffuse : public BxDF
{
    Spectrum albedo_;

public:

    explicit BxDF_Diffuse(const Spectrum &albedo) noexcept;

    Spectrum GetBaseColor() const noexcept override;

    Spectrum EvalUncolored(const Vec3 &wi, const Vec3 &wo, bool star) const noexcept override;

    std::optional<SampleWiResult> SampleWi(const Vec3 &wo, bool star, const Vec3 &sample) const noexcept override;

    Real SampleWiPDF(const Vec3 &wi, const Vec3 &wo, bool star) const noexcept override;
};

} // namespace Atrc
