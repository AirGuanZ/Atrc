#pragma once

#include <vector>

#include <Atrc/Core/Core.h>
#include <Atrc/Material/BxDF/BxDF.h>

AGZ_NS_BEG(Atrc)

class BxDFAggregate : public BSDF
{
    static constexpr size_t MAX_BXDF_CNT = 8;

    const BxDF *bxdfs_[MAX_BXDF_CNT];
    size_t bxdfCnt_;

public:

    BxDFAggregate(const LocalCoordSystem &shdLocal, const LocalCoordSystem &geoLocal);

    void AddBxDF(const BxDF *bxdf);

    Spectrum Eval(const Vec3 &wi, const Vec3 &wo, BxDFType type) const override;

    Option<BSDFSampleWiResult> SampleWi(const Vec3 &wo, BxDFType type) const override;

    Real SampleWiPDF(const Vec3 &wi, const Vec3 &wo, BxDFType type) const override;
};

AGZ_NS_END(Atrc)
