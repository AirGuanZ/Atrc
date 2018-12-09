#pragma once

#include <Atrc/Material/BxDF/BxDF.h>

AGZ_NS_BEG(Atrc)

class BxDFScaler : public BxDF
{
    Spectrum scale_;
    BxDF *bxdf_;

public:

    BxDFScaler(const Spectrum &scale, BxDF *bxdf);

    Spectrum Eval(const LocalCoordSystem &localShdCoord, const Vec3 &wi, const Vec3 &wo) const override;

    Option<BxDFSampleWiResult> SampleWi(const LocalCoordSystem &localShdCoord, const Vec3 &wo) const override;

    Real SampleWiPDF(const LocalCoordSystem &localShdCoord, const Vec3 &wi, const Vec3 &wo) const override;
};

AGZ_NS_END(Atrc)
