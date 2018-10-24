#include <Atrc/Material/BxDF/BxDFScaler.h>

AGZ_NS_BEG(Atrc)

BxDFScaler::BxDFScaler(const Spectrum &scale, RC<BxDF> bxdf)
    : BxDF(bxdf->GetType()), scale_(scale)
{
    AGZ_ASSERT(bxdf);
    bxdf_ = std::move(bxdf);
}

Spectrum BxDFScaler::Eval(const Vec3 &wi, const Vec3 &wo) const
{
    return scale_ * bxdf_->Eval(wi, wo);
}

Option<BxDFSampleWiResult> BxDFScaler::SampleWi(const Vec3 &wo) const
{
    Option<BxDFSampleWiResult> ret = bxdf_->SampleWi(wo);
    if(ret)
        ret->coef *= scale_;
    return ret;
}

Real BxDFScaler::SampleWiPDF(const Vec3 &wi, const Vec3 &wo) const
{
    return bxdf_->SampleWiPDF(wi, wo);
}

AGZ_NS_END(Atrc)
