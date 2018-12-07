#include <Atrc/Material/BxDF/PerfectSpecularReflection.h>
#include <Atrc/Material/BxDFAggregate.h>
#include <Atrc/Material/IdealMirror.h>

AGZ_NS_BEG(Atrc)

IdealMirror::IdealMirror(const Spectrum &rc, const Fresnel *fresnel)
    : rc_(rc), fresnel_(fresnel)
{

}

void IdealMirror::Shade(const SurfacePoint &sp, ShadingPoint *dst, AGZ::ObjArena<> &arena) const
{
    AGZ_ASSERT(dst);

    auto bsdf = arena.Create<BxDFAggregate>(sp.geoLocal.ez, sp.geoLocal);
    bsdf->AddBxDF(arena.Create<PerfectSpecularReflection>(rc_, fresnel_));

    dst->normal = sp.geoLocal.ez;
    dst->bsdf = bsdf;
}

AGZ_NS_END(Atrc)
