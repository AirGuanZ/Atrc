#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

using BxDFSampleWiResult = BSDFSampleWiResult;

class BxDF
{
protected:

    BxDFType type_;

public:

    virtual ~BxDF() = default;

    explicit BxDF(BxDFType type);

    bool MatchType(BxDFType type) const;

    virtual Spectrum Eval(const Vec3 &wi, const Vec3 &wo) const = 0;

    virtual Option<BxDFSampleWiResult> SampleWi(const Vec3 &wo, BxDFType type) const = 0;
};

inline BxDF::BxDF(BxDFType type)
    : type_(type)
{

}

inline bool BxDF::MatchType(BxDFType type) const
{
    return (type_ & type) == type_;
}

AGZ_NS_END(Atrc)
