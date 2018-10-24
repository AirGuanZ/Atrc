#include <Atrc/Material/BxDF/PerfectSpecularReflection.h>
#include <Atrc/Material/BxDFAggregate.h>
#include <Atrc/Material/IdealMirror.h>

AGZ_NS_BEG(Atrc)

IdealMirror::IdealMirror(const Spectrum &rc, RC<const Fresnel> fresnel)
    : rc_(rc), fresnel_(std::move(fresnel))
{

}

void IdealMirror::Shade(const SurfacePoint &sp, ShadingPoint *dst) const
{
    AGZ_ASSERT(dst);

    auto bsdf = MakeRC<BxDFAggregate>(sp.geoLocal, sp.geoLocal);
    bsdf->AddBxDF(MakeRC<PerfectSpecularReflection>(rc_, fresnel_));

    dst->shdLocal = sp.geoLocal;
    dst->bsdf = std::move(bsdf);
}

AGZ_NS_END(Atrc)
