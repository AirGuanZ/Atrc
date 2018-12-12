#pragma once

#include <Atrc/Lib/Core/Material.h>

namespace Atrc
{

class BxDF
{
protected:

    const BSDFType type_;

public:

    using SampleWiResult = BSDF::SampleWiResult;

    virtual ~BxDF() = default;

    explicit BxDF(BSDFType type) noexcept;

    BSDFType GetType() const noexcept;

    bool MatchType(BSDFType type) const noexcept;

    virtual Spectrum GetAlbedo() const noexcept = 0;

    virtual Spectrum Eval(const CoordSystem &geoInShd, const Vec3 &wi, const Vec3 &wo) const noexcept = 0;

    virtual Option<SampleWiResult> SampleWi(const CoordSystem &geoInShd, const Vec3 &wo, const Vec2 &sample) const noexcept = 0;

    virtual Real SampleWiPDF(const CoordSystem &geoInShd, const Vec3 &wi, const Vec3 &wo) const noexcept = 0;
};

// ================================= Implementation

inline BxDF::BxDF(BSDFType type) noexcept
    : type_(type)
{
    
}

inline BSDFType BxDF::GetType() const noexcept
{
    return type_;
}

inline bool BxDF::MatchType(BSDFType type) const noexcept
{
    return Contains(type, GetType());
}

} // namespace Atrc
