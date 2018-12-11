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

    virtual Option<SampleWiResult> SampleWi(const CoordSystem &geoInShd, const Vec3 &wo) const noexcept = 0;

    virtual Real SampleWiPDF(const CoordSystem &geoInShd, const Vec3 &wi, const Vec3 &wo) const noexcept = 0;
};

// ================================= Implementation

BxDF::BxDF(BSDFType type) noexcept
    : type_(type)
{
    AGZ_ASSERT(Contains(type, BSDF_DIFFUSE) ||
               Contains(type, BSDF_GLOSSY) ||
               Contains(type, BSDF_SPECULAR));
    AGZ_ASSERT(Contains(type, BSDF_REFLECTION) ||
               Contains(type, BSDF_TRANSMISSION));
}

BSDFType BxDF::GetType() const noexcept
{
    return type_;
}

bool BxDF::MatchType(BSDFType type) const noexcept
{
    return Contains(type, GetType());
}

} // namespace Atrc
