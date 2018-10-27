#pragma once

#include <Atrc/Material/BxDF/TorranceSparrow.h>

AGZ_NS_BEG(Atrc)

class BlinnPhongDistribution : public MicrofacetDistribution
{
    Real e_;

public:

    explicit BlinnPhongDistribution(Real e);

    float Eval(const Vec3 &H) const override;

    Option<MDSampleResult> SampleWi(const Vec3 &wo) const override;

    Real SampleWiPDF(const Vec3 &wi, const Vec3 &wo) const override;
};

AGZ_NS_END(Atrc)
