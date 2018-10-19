#include <Atrc/Core/BSDF.h>
#include <Atrc/Material/BlackMaterial.h>

AGZ_NS_BEG(Atrc)

namespace
{
    class BlackBSDF : public BSDF
    {
    public:

        BlackBSDF(const LocalCoordSystem &shadingLocal, const Vec3 &geometryNormal)
            : BSDF(shadingLocal, geometryNormal)
        {
            
        }

        Spectrum Eval(const Vec3 &wi, const Vec3 &wo) const override
        {
            return SPECTRUM::BLACK;
        }

        Option<BSDFSampleWiResult> SampleWi(const Vec3 &wo, BxDFType type) const override
        {
            return None;
        }

        Real SampleWiPDF(const Vec3 &wi, const Vec3 &wo, BxDFType type) const override
        {
            return 0.0;
        }
    };
}

void BlackMaterial::Shade(const SurfacePoint &sp, ShadingPoint *dst) const
{
    AGZ_ASSERT(dst);
    dst->bsdf = MakeRC<BlackBSDF>(*sp.shdLocal, sp.geoLocal.ez);
}

AGZ_NS_END(Atrc)
