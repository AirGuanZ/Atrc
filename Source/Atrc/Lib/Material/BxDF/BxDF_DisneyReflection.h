#pragma once

#include <Atrc/Lib/Material/Utility/BxDF.h>

namespace Atrc
{
    
class BxDF_DisneyReflection : public BxDF
{
    Spectrum baseColor_;
    Real metallic_;
    Real subsurface_;
    Real specular_;
    Real specularTint_;
    Real roughness_;
    Real anisotropic_;
    Real sheen_;
    Real sheenTint_;
    Real clearcoat_;
    Real clearcoatGloss_;

    Real ax_, ay_;
    Real clearcoatRoughness_;

    struct SampleWeights { Real wd, ws, wc; } sampleWeights_;

public:

    BxDF_DisneyReflection(
        const Spectrum &baseColor,
        Real metallic,
        Real subsurface,
        Real specular,
        Real specularTint,
        Real roughness,
        Real anisotropic,
        Real sheen,
        Real sheenTint,
        Real clearcoat,
        Real clearcoatGloss) noexcept;

    Spectrum GetBaseColor() const noexcept override { return baseColor_; }

    Spectrum Eval(const Vec3 &wi, const Vec3 &wo, bool star) const noexcept override;

    std::optional<SampleWiResult> SampleWi(const Vec3 &wo, bool star, const Vec3 &sample) const noexcept override;

    Real SampleWiPDF(const Vec3 &wi, const Vec3 &wo, bool star) const noexcept override;
};

} // namespace Atrc
