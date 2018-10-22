#include <Atrc/Material/BxDF/DiffuseBRDF.h>

AGZ_NS_BEG(Atrc)

DiffuseBRDF::DiffuseBRDF(const Spectrum &color)
    : BxDF(BXDF_DIFFUSE | BXDF_REFLECTION), color_(color)
{

}

Spectrum DiffuseBRDF::Eval(const Vec3 &wi, const Vec3 &wo) const
{
    if(wi.z <= 0.0 || wo.z <= 0.0)
        return Spectrum();
    return color_;
}

Option<BxDFSampleWiResult> DiffuseBRDF::SampleWi(const Vec3 &wo) const
{
    AGZ_ASSERT(IsNormalized(wo));

    if(wo.z <= 0.0)
        return None;

    Real u = Rand(), v = Rand();
    auto [sam, pdf] = AGZ::Math::DistributionTransform
        ::ZWeightedOnUnitHemisphere<Real>::Transform({ u, v });

    BxDFSampleWiResult ret;

    ret.wi   = sam;
    ret.pdf  = pdf;
    ret.coef = color_;
    ret.type = type_;

    return ret;
}

Real DiffuseBRDF::SampleWiPDF(const Vec3 &wi, const Vec3 &wo) const
{
    AGZ_ASSERT(IsNormalized(wi));
    AGZ_ASSERT(IsNormalized(wo));

    if(wi.z <= 0.0 || wo.z <= 0.0)
        return 0.0;

    return AGZ::Math::DistributionTransform
        ::ZWeightedOnUnitHemisphere<Real>::PDF(wi);
}

AGZ_NS_END(Atrc)
