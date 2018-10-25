#pragma once

#include <Atrc/Material/BxDF/BxDF.h>
#include <Atrc/Material/Fresnel.h>

AGZ_NS_BEG(Atrc)

class PerfectSpecular : public BxDF
{
    Spectrum rc_;
    const FresnelDielectric *fresnel_;

public:

    PerfectSpecular(const Spectrum &rc, const FresnelDielectric *fresnel);

    Spectrum Eval(const Vec3 &wi, const Vec3 &wo) const override;

    Option<BxDFSampleWiResult> SampleWi(const Vec3 &wo) const override;

    Real SampleWiPDF(const Vec3 &wi, const Vec3 &wo) const override;
};

AGZ_NS_END(Atrc)
