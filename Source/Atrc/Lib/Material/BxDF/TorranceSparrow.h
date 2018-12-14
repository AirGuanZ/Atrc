#pragma once

#include <Atrc/Lib/Material/Utility/BxDF.h>
#include <Atrc/Lib/Material/Utility/Fresnel.h>

namespace Atrc
{
    
class MicrofacetDistribution
{
public:

    virtual ~MicrofacetDistribution() = default;

    virtual Real D(const Vec3 &H) const noexcept = 0;

    virtual Real G(const Vec3 &H, const Vec3 &wi, const Vec3 &wo) const noexcept = 0;

    struct SampleWiResult
    {
        Vec3 wi;
        Real pdf;
    };

    virtual Option<SampleWiResult> SampleWi(const CoordSystem &geoInShd, const Vec3 &wo, const Vec2 &sample) const noexcept = 0;

    virtual Real SampleWiPDF(const CoordSystem &geoInShd, const Vec3 &wi, const Vec3 &wo) const noexcept = 0;
};

class TorranceSparrow : public BxDF
{
    Spectrum rc_;
    const MicrofacetDistribution *md_;
    const Fresnel *fresnel_;

public:

    TorranceSparrow(const Spectrum &rc, const MicrofacetDistribution *md, const Fresnel *fresnel) noexcept;

    Spectrum GetAlbedo() const noexcept override;

    Spectrum Eval(const CoordSystem &geoInShd, const Vec3 &wi, const Vec3 &wo) const noexcept override;

    Option<SampleWiResult> SampleWi(const CoordSystem &geoInShd, const Vec3 &wo, const Vec2 &sample) const noexcept override;

    Real SampleWiPDF(const CoordSystem &geoInShd, const Vec3 &wi, const Vec3 &wo) const noexcept override;
};

} // namespace Atrc
