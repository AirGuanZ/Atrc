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

    Spectrum GetAlbedo() const noexcept override;

    Spectrum Eval(const CoordSystem &geoInShd, const Vec3 &wi, const Vec3 &wo, bool star) const noexcept override;

    std::optional<SampleWiResult> SampleWi(const CoordSystem &geoInShd, const Vec3 &wo, bool star, const Vec2 &sample) const noexcept override;

    Real SampleWiPDF(const CoordSystem &geoInShd, const Vec3 &wi, const Vec3 &wo, bool star) const noexcept override;
};

} // namespace Atrc
