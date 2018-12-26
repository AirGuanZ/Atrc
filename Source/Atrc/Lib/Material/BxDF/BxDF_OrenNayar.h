#pragma once

#include <Atrc/Lib/Material/Utility/BxDF.h>

namespace Atrc
{
    
class BxDF_OrenNayar : public BxDF
{
    Spectrum albedo_;
    Real A_, B_;

public:

    BxDF_OrenNayar(const Spectrum &albedo, Real sigma) noexcept;

    Spectrum GetAlbedo() const noexcept override;

    Spectrum Eval(const CoordSystem &geoInShd, const Vec3 &wi, const Vec3 &wo, bool star) const noexcept override;
};

} // namespace Atrc
