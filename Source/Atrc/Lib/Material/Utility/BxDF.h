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

    virtual Spectrum Eval(const CoordSystem &geoInShd, const Vec3 &wi, const Vec3 &wo, bool star) const noexcept = 0;

    virtual Option<SampleWiResult> SampleWi(const CoordSystem &geoInShd, const Vec3 &wo, bool star, const Vec2 &sample) const noexcept;

    virtual Real SampleWiPDF(const CoordSystem &geoInShd, const Vec3 &wi, const Vec3 &wo, bool star) const noexcept;
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

inline Option<BxDF::SampleWiResult> BxDF::SampleWi(const CoordSystem &geoInShd, const Vec3 &wo, bool star, const Vec2 &sample) const noexcept
{
    if(wo.z <= 0 || !geoInShd.InPositiveHemisphere(wo))
        return None;

    auto[sam, pdf] = AGZ::Math::DistributionTransform
        ::ZWeightedOnUnitHemisphere<Real>::Transform(sample);
    if(!geoInShd.InPositiveHemisphere(sam) || !pdf)
        return None;

    SampleWiResult ret;
    ret.coef    = Eval(geoInShd, sam, wo, star);
    ret.pdf     = pdf;
    ret.type    = type_;
    ret.wi      = sam;
    ret.isDelta = false;
    return ret;
}

inline Real BxDF::SampleWiPDF(const CoordSystem &geoInShd, const Vec3 &wi, const Vec3 &wo, [[maybe_unused]] bool star) const noexcept
{
    if(wi.z <= 0 || wo.z <= 0 || !geoInShd.InPositiveHemisphere(wi) || !geoInShd.InPositiveHemisphere(wo))
        return 0;
    return AGZ::Math::DistributionTransform::ZWeightedOnUnitHemisphere<Real>::PDF(wi.Normalize());
}

} // namespace Atrc
