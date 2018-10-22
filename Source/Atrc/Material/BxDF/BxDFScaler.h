#pragma once

#include <Atrc/Material/BxDF/BxDF.h>

AGZ_NS_BEG(Atrc)

class BxDFScaler : public BxDF
{
    Spectrum scale_;
    RC<BxDF> bxdf_;

public:

    BxDFScaler(BxDFType type, const Spectrum &scale, RC<BxDF> bxdf);

    Spectrum Eval(const Vec3 &wi, const Vec3 &wo) const override;

    Option<BxDFSampleWiResult> SampleWi(const Vec3 &wo, BxDFType type) const override;
};

AGZ_NS_END(Atrc)
