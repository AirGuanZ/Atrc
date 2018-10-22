#include <Atrc/Material/PureDiffuse.h>

AGZ_NS_BEG(Atrc)

namespace
{
    class PureDiffuseBSDF : public BSDF
    {
        Spectrum color_;

    public:

        explicit PureDiffuseBSDF(const LocalCoordSystem &shdLocal, const LocalCoordSystem &geoLocal, const Spectrum &color)
            : BSDF(shdLocal, geoLocal), color_(color)
        {
            
        }

        Spectrum Eval(const Vec3 &wi, const Vec3 &wo) const override
        {
            AGZ_ASSERT(IsNormalized(wi));
            AGZ_ASSERT(IsNormalized(wo));

            if(shadingLocal_.World2Local(wi).z > 0.0 && shadingLocal_.World2Local(wo).z > 0.0)
                return color_;
            return SPECTRUM::BLACK;
        }

        Option<BSDFSampleWiResult> SampleWi(const Vec3 &wo, BxDFType type) const override
        {
            AGZ_ASSERT(IsNormalized(wo));

            if((type & (BXDF_DIFFUSE | BXDF_REFLECTION)) != (BXDF_DIFFUSE | BXDF_REFLECTION))
                return None;

            Vec3 localWo = shadingLocal_.World2Local(wo);
            if(localWo.z <= 0.0)
                return None;

            Real u = Rand(), v = Rand();
            auto [sam, pdf] = AGZ::Math::DistributionTransform
                ::ZWeightedOnUnitHemisphere<Real>::Transform({ u, v });

            BSDFSampleWiResult ret;
            ret.wi   = shadingLocal_.Local2World(sam);
            ret.pdf  = pdf;
            ret.coef = color_;
            ret.type = BxDFType(BXDF_DIFFUSE | BXDF_REFLECTION);

            AGZ_ASSERT(ApproxEq(ret.pdf, SampleWiPDF(ret.wi, wo, type), 1e-5));

            return ret;
        }

        Real SampleWiPDF(const Vec3 &wi, const Vec3 &wo, BxDFType type) const override
        {
            AGZ_ASSERT(IsNormalized(wi));
            AGZ_ASSERT(IsNormalized(wo));

            if(!(BXDF_DIFFUSE & type))
                return 0.0;

            Vec3 localWi = shadingLocal_.World2Local(wi);
            if(localWi.z <= 0.0)
                return 0.0;

            return AGZ::Math::DistributionTransform
                ::ZWeightedOnUnitHemisphere<Real>::PDF(localWi);
        }
    };
}

PureDiffuse::PureDiffuse(const Spectrum &albedo)
    : color_(AGZ::Math::InvPI<float> * albedo)
{

}

void PureDiffuse::Shade(const SurfacePoint &sp, ShadingPoint *dst) const
{
    AGZ_ASSERT(dst);
    dst->shdLocal = sp.geoLocal;
    dst->bsdf = MakeRC<PureDiffuseBSDF>(dst->shdLocal, sp.geoLocal, color_);
}

AGZ_NS_END(Atrc)
