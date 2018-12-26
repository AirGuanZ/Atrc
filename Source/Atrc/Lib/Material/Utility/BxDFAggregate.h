#pragma once

#include <Atrc/Lib/Core/Material.h>
#include <Atrc/Lib/Material/Utility/BxDF.h>

namespace Atrc
{

template<uint8_t MAX_BXDF_CNT = 8>
class BxDFAggregate : public BSDF
{
    const BxDF *bxdfs_[MAX_BXDF_CNT];
    uint8_t bxdfCnt_;

    // 在着色TBN中的几何TBN
    const CoordSystem geoInShd_;

public:

    BxDFAggregate(const CoordSystem &shd, const CoordSystem &geo) noexcept;

    void AddBxDF(const BxDF *bxdf) noexcept;

    Spectrum GetAlbedo(BSDFType type) const noexcept override;

    Spectrum Eval(
        const CoordSystem &shd, const CoordSystem &geo,
        const Vec3 &wi, const Vec3 &wo, BSDFType type, bool star) const noexcept override;
    
    Option<SampleWiResult> SampleWi(
        const CoordSystem &shd, const CoordSystem &geo,
        const Vec3 &wo, BSDFType type, bool star, const Vec2 &sample) const noexcept override;

    Real SampleWiPDF(
        const CoordSystem &shd, const CoordSystem &geo,
        const Vec3 &wi, const Vec3 &wo, BSDFType type, bool star) const noexcept override;
};

// ================================= Implementation

template<uint8_t MAX_BXDF_CNT>
BxDFAggregate<MAX_BXDF_CNT>::BxDFAggregate(
    const CoordSystem &shd, const CoordSystem &geo) noexcept
    : bxdfCnt_(0), geoInShd_(shd.World2Local(geo))
{

}

template<uint8_t MAX_BXDF_CNT>
void BxDFAggregate<MAX_BXDF_CNT>::AddBxDF(const BxDF *bxdf) noexcept
{
    AGZ_ASSERT(bxdfCnt_ < MAX_BXDF_CNT);
    AGZ_ASSERT(bxdf);
    bxdfs_[bxdfCnt_++] = bxdf;
}

template<uint8_t MAX_BXDF_CNT>
Spectrum BxDFAggregate<MAX_BXDF_CNT>::GetAlbedo(BSDFType type) const noexcept
{
    Spectrum ret;
    for(uint8_t i = 0; i < bxdfCnt_; ++i)
    {
        if(bxdfs_[i]->MatchType(type))
            ret += bxdfs_[i]->GetAlbedo();
    }
    return ret;
}

template<uint8_t MAX_BXDF_CNT>
Spectrum BxDFAggregate<MAX_BXDF_CNT>::Eval(
    const CoordSystem &shd, [[maybe_unused]] const CoordSystem &geo,
    const Vec3 &wi, const Vec3 &wo, BSDFType type, bool star) const noexcept
{
    Vec3 lwi = shd.World2Local(wi), lwo = shd.World2Local(wo);
    Spectrum ret;
    for(uint8_t i = 0; i < bxdfCnt_; ++i)
    {
        if(bxdfs_[i]->MatchType(type))
            ret += bxdfs_[i]->Eval(geoInShd_, lwi, lwo, star);
    }
    return ret;
}

template<uint8_t MAX_BXDF_CNT>
Option<typename BxDFAggregate<MAX_BXDF_CNT>::SampleWiResult>
BxDFAggregate<MAX_BXDF_CNT>::SampleWi(
    const CoordSystem &shd, [[maybe_unused]] const CoordSystem &geo,
    const Vec3 &wo, BSDFType type, bool star, const Vec2 &sample) const noexcept
{
    // 有多少bxdf的type对得上

    uint8_t nMatched = 0;
    for(uint8_t i = 0; i < bxdfCnt_; ++i)
        nMatched += bxdfs_[i]->MatchType(type);
    if(!nMatched)
        return None;
    
    // 从sample中抽取一个整数用来选择bxdf
    
    auto [dstIdx, newSampleX] = AGZ::Math::DistributionTransform
        ::SampleExtractor<Real>::ExtractInteger<uint8_t>(sample.x, 0, nMatched);
    Vec2 newSample(newSampleX, sample.y);
    
    // 选出bxdf

    const BxDF *bxdf = nullptr;
    for(uint8_t i = 0, j = 0; i < bxdfCnt_; ++i)
    {
        if(bxdfs_[i]->MatchType(type) && j++ == dstIdx)
        {
            bxdf = bxdfs_[i];
            break;
        }
    }

    // 采样选出的bxdf

    Vec3 lwo = shd.World2Local(wo);
    auto ret = bxdf->SampleWi(geoInShd_, lwo, star, newSample);
    if(!ret)
        return None;
    
    // 如果是delta，那么不需要计入别的bxdf

    if(ret->isDelta || nMatched <= 1)
    {
        ret->pdf /= nMatched;
        ret->wi = shd.Local2World(ret->wi);
        return ret;
    }

    // 综合考虑所有bxdf的意见

    for(uint8_t i = 0; i < bxdfCnt_; ++i)
    {
        if(!bxdfs_[i]->MatchType(type) || bxdfs_[i] == bxdf)
            continue;
        ret->pdf += bxdfs_[i]->SampleWiPDF(geoInShd_, ret->wi, lwo, star);
        ret->coef += bxdfs_[i]->Eval(geoInShd_, ret->wi, lwo, star);
    }

    ret->pdf /= nMatched;
    ret->wi = shd.Local2World(ret->wi);
    return ret;
}

template<uint8_t MAX_BXDF_CNT>
Real BxDFAggregate<MAX_BXDF_CNT>::SampleWiPDF(
    const CoordSystem &shd, [[maybe_unused]] const CoordSystem &geo,
        const Vec3 &wi, const Vec3 &wo, BSDFType type, bool star) const noexcept
{
    Vec3 lwi = shd.World2Local(wi), lwo = shd.World2Local(wo);
    uint8_t nMatched = 0; Real pdf = 0;
    for(uint8_t i = 0; i < bxdfCnt_; ++i)
    {
        if(bxdfs_[i]->MatchType(type))
        {
            ++nMatched;
            pdf += bxdfs_[i]->SampleWiPDF(geoInShd_, lwi, lwo, star);
        }
    }
    return nMatched ? pdf / nMatched : Real(0);
}

} // namespace Atrc
