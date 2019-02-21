#pragma once

#include <Atrc/Lib/Core/CoordSystem.h>
#include <Atrc/Lib/Material/Utility/MicrofacetDistribution.h>

namespace Atrc
{
    
class BlinnPhong : public MicrofacetDistribution
{
    Real e_;

public:

    explicit BlinnPhong(Real e) noexcept;

    Real D(const Vec3 &H) const noexcept override;

    Real G(const Vec3 &H, const Vec3 &wi, const Vec3 &wo) const noexcept override;

    std::optional<SampleWiResult> SampleWi(const Vec3 &wo, const Vec2 &sample) const noexcept override;

    // wi和wo必须在geo coord、shd coord中均处于正半平面
    Real SampleWiPDF(const Vec3 &wi, const Vec3 &wo) const noexcept override;
};

} // namespace Atrc
