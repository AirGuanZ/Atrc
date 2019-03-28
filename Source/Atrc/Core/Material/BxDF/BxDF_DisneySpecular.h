#pragma once

#include <Atrc/Core/Material/Utility/BxDF.h>

namespace Atrc
{
    
class BxDF_DisneySpecular : public BxDF
{
    Spectrum baseColor_;
    Real specular_;
    Real specularTint_;
    Real metallic_;
    Real roughness_;
    Real anisotropic_;
    Real ax_, ay_;

public:

    BxDF_DisneySpecular(
        const Spectrum &baseColor,
        Real specular,
        Real specularTint,
        Real metallic,
        Real roughness,
        Real anisotropic) noexcept;

    Spectrum GetBaseColor() const noexcept override { return baseColor_; }

    Spectrum Eval(const Vec3 &wi, const Vec3 &wo, bool star) const noexcept override;

    // 实现不了符合约束的eval uncolored，故此函数无意义
    Spectrum EvalUncolored(const Vec3 &wi, const Vec3 &wo, bool star) const noexcept override { return Spectrum(); }

    std::optional<SampleWiResult> SampleWi(const Vec3 &wo, bool star, const Vec3 &sample) const noexcept override;

    Real SampleWiPDF(const Vec3 &wi, const Vec3 &wo, bool star) const noexcept override;
};

} // namespace Atrc
