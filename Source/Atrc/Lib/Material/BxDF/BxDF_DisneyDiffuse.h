#pragma once

#include <Atrc/Lib/Material/Utility/BxDF.h>

namespace Atrc
{

class BxDF_DisneyDiffuse : public BxDF
{
    Spectrum baseColor_;
    Real subsurface_;
    Real roughness_;

public:

    BxDF_DisneyDiffuse(const Spectrum &baseColor, Real subsurface, Real roughness) noexcept;

    Spectrum GetAlbedo() const noexcept override;

    Spectrum Eval(const Vec3 &wi, const Vec3 &wo, bool star) const noexcept override;
};

} // namespace Atrc
