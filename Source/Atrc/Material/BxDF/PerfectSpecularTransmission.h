#pragma once

#include <Atrc/Material/BxDF/BxDF.h>
#include <Atrc/Material/Fresnel.h>

AGZ_NS_BEG(Atrc)

class PerfectSpecularTransmission : public BxDF
{
    Spectrum rc_;
    RC<const Fresnel> fresnel_;

public:

    PerfectSpecularTransmission(const Spectrum &rc, RC<const Fresnel> fresnel);

    Spectrum Eval(const Vec3 &wi, const Vec3 &wo) const override;

    Option<BxDFSampleWiResult> SampleWi(const Vec3 &wo) const override;

    Real SampleWiPDF(const Vec3 &wi, const Vec3 &wo) const override;
};

AGZ_NS_END(Atrc)
