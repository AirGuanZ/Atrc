#include <Atrc/Material/BxDF/BxDFScaler.h>

AGZ_NS_BEG(Atrc)

BxDFScaler::BxDFScaler(const Spectrum &scale, BxDF *bxdf)
    : BxDF(bxdf->GetType()), scale_(scale)
{
    AGZ_ASSERT(bxdf);
    bxdf_ = bxdf;
}

Spectrum BxDFScaler::Eval(const LocalCoordSystem &localShdCoord, const Vec3 &wi, const Vec3 &wo) const
{
    return scale_ * bxdf_->Eval(localShdCoord, wi, wo);
}

Option<BxDFSampleWiResult> BxDFScaler::SampleWi(const LocalCoordSystem &localShdCoord, const Vec3 &wo) const
{
    Option<BxDFSampleWiResult> ret = bxdf_->SampleWi(localShdCoord, wo);
    if(ret)
        ret->coef *= scale_;
    return ret;
}

Real BxDFScaler::SampleWiPDF(const LocalCoordSystem &localShdCoord, const Vec3 &wi, const Vec3 &wo) const
{
    return bxdf_->SampleWiPDF(localShdCoord, wi, wo);
}

AGZ_NS_END(Atrc)
