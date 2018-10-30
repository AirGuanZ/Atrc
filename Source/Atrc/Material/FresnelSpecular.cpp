#include <Atrc/Material/BxDF/PerfectSpecularReflection.h>
#include <Atrc/Material/BxDF/PerfectSpecularTransmission.h>
#include <Atrc/Material/BxDF/PerfectSpecular.h>
#include <Atrc/Material/BxDFAggregate.h>
#include <Atrc/Material/FresnelSpecular.h>

AGZ_NS_BEG(Atrc)

FresnelSpecular::FresnelSpecular(const Spectrum &rc, const Dielectric *fresnel)
    : rc_(rc), fresnel_(fresnel)
{
    AGZ_ASSERT(fresnel_);
}

Material *FresnelSpecular::Clone(const SceneParamGroup &group, AGZ::ObjArena<> &arena) const
{
    return arena.Create<FresnelSpecular>(rc_, fresnel_);
}

void FresnelSpecular::Shade(const SurfacePoint &sp, ShadingPoint *dst, AGZ::ObjArena<> &arena) const
{
    AGZ_ASSERT(dst);

    auto bsdf = arena.Create<BxDFAggregate>(sp.geoLocal, sp.geoLocal);
    bsdf->AddBxDF(arena.Create<PerfectSpecular>(rc_, fresnel_));

    dst->bsdf = bsdf;
    dst->shdLocal = sp.geoLocal;
}

AGZ_NS_END(Atrc)
