#include <Atrc/Material/BlackMaterial.h>

AGZ_NS_BEG(Atrc)

namespace
{
    class BlackBSDF : public BSDF
    {
    public:

        BlackBSDF(const LocalCoordSystem &shadingLocal, const LocalCoordSystem &geoLocal)
            : BSDF(shadingLocal, geoLocal)
        {
            
        }

        Spectrum Eval(const Vec3 &wi, const Vec3 &wo, [[maybe_unused]] BxDFType type) const override
        {
            AGZ_ASSERT(IsNormalized(wi));
            AGZ_ASSERT(IsNormalized(wo));
            return SPECTRUM::BLACK;
        }

        Option<BSDFSampleWiResult> SampleWi(const Vec3 &wo, [[maybe_unused]] BxDFType type) const override
        {
            AGZ_ASSERT(IsNormalized(wo));
            return None;
        }

        Real SampleWiPDF(const Vec3 &wi, const Vec3 &wo, [[maybe_unused]] BxDFType type) const override
        {
            AGZ_ASSERT(IsNormalized(wi));
            AGZ_ASSERT(IsNormalized(wo));
            return 0.0;
        }
    };
}

void BlackMaterial::Shade(const SurfacePoint &sp, ShadingPoint *dst, AGZ::ObjArena<> &arena) const
{
    AGZ_ASSERT(dst);
    dst->shdLocal = sp.geoLocal;

    dst->bsdf = arena.Create<BlackBSDF>(dst->shdLocal, sp.geoLocal);
}

AGZ_NS_END(Atrc)
