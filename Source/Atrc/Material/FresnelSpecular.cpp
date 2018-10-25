#include <Atrc/Material/BxDF/PerfectSpecularReflection.h>
#include <Atrc/Material/BxDF/PerfectSpecularTransmission.h>
#include <Atrc/Material/BxDF/PerfectSpecular.h>
#include <Atrc/Material/BxDFAggregate.h>
#include <Atrc/Material/FresnelSpecular.h>

AGZ_NS_BEG(Atrc)

FresnelSpecular::FresnelSpecular(const Spectrum &rc, RC<const FresnelDielectric> fresnel)
    : rc_(rc), fresnel_(std::move(fresnel))
{
    AGZ_ASSERT(fresnel_);
}

void FresnelSpecular::Shade(const SurfacePoint &sp, ShadingPoint *dst, AGZ::ObjArena<> &arena) const
{
    AGZ_ASSERT(dst);

    auto bsdf = arena.Create<BxDFAggregate>(sp.geoLocal, sp.geoLocal);
    bsdf->AddBxDF(arena.Create<PerfectSpecular>(rc_, fresnel_.get()));

    dst->bsdf = bsdf;
    dst->shdLocal = sp.geoLocal;
}

AGZ_NS_END(Atrc)
