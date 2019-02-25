#pragma once

#include <Atrc/Lib/Core/Material.h>

namespace Atrc
{

// 所有方向输入都应是本地归一化的
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

    virtual Spectrum GetBaseColor() const noexcept = 0;

    virtual Spectrum Eval(const Vec3 &wi, const Vec3 &wo, bool star) const noexcept = 0;

    virtual std::optional<SampleWiResult> SampleWi(const Vec3 &wo, bool star, const Vec3 &sample) const noexcept;

    virtual Real SampleWiPDF(const Vec3 &wi, const Vec3 &wo, bool star) const noexcept;
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

inline std::optional<BxDF::SampleWiResult> BxDF::SampleWi(const Vec3 &wo, bool star, const Vec3 &sample) const noexcept
{
    if(wo.z <= 0)
        return std::nullopt;

    auto [sam, pdf] = AGZ::Math::DistributionTransform
        ::ZWeightedOnUnitHemisphere<Real>::Transform(sample.xy());
    if(sam.z <= 0 || !pdf)
        return std::nullopt;

    SampleWiResult ret;
    ret.coef    = Eval(sam, wo, star);
    ret.pdf     = pdf;
    ret.type    = type_;
    ret.wi      = sam;
    ret.isDelta = false;
    return ret;
}

inline Real BxDF::SampleWiPDF(const Vec3 &wi, const Vec3 &wo, [[maybe_unused]] bool star) const noexcept
{
    if(wi.z <= 0 || wo.z <= 0)
        return 0;
    return AGZ::Math::DistributionTransform::ZWeightedOnUnitHemisphere<Real>::PDF(wi);
}

} // namespace Atrc
