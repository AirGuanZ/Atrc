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

    template<typename T>
    explicit BxDF(T type) : type_(BxDFType(type)) { }

    BxDFType GetType() const { return type_; }

    bool MatchType(BxDFType type) const { return (type_ & type) == type_; }

    virtual Spectrum Eval(const Vec3 &wi, const Vec3 &wo) const = 0;

    virtual Option<BxDFSampleWiResult> SampleWi(const Vec3 &wo) const = 0;

    virtual Real SampleWiPDF(const Vec3 &wi, const Vec3 &wo) const = 0;
};

AGZ_NS_END(Atrc)
