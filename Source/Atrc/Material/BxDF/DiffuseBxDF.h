#pragma once

#include <Atrc/Material/BxDF/BxDF.h>

AGZ_NS_BEG(Atrc)

class DiffuseBxDF : public BxDF
{
    Spectrum color_;

public:

    explicit DiffuseBxDF(const Spectrum &color);

    Spectrum Eval(const Vec3 &wi, const Vec3 &wo) const override;

    Option<BxDFSampleWiResult> SampleWi(const Vec3 &wo) const override;

    Real SampleWiPDF(const Vec3 &wi, const Vec3 &wo) const override;
};

AGZ_NS_END(Atrc)
