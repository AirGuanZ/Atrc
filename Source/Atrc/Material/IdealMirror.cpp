#include <Atrc/Material/BxDF/PerfectSpecularReflection.h>
#include <Atrc/Material/BxDFAggregate.h>
#include <Atrc/Material/IdealMirror.h>

AGZ_NS_BEG(Atrc)

IdealMirror::IdealMirror(const Spectrum &rc, RC<const Fresnel> fresnel)
    : rc_(rc), fresnel_(std::move(fresnel))
{

}

void IdealMirror::Shade(const SurfacePoint &sp, ShadingPoint *dst, AGZ::ObjArena<> &arena) const
{
    AGZ_ASSERT(dst);

    auto bsdf = arena.Create<BxDFAggregate>(sp.geoLocal, sp.geoLocal);
    bsdf->AddBxDF(arena.Create<PerfectSpecularReflection>(rc_, fresnel_.get()));

    dst->shdLocal = sp.geoLocal;
    dst->bsdf = bsdf;
}

AGZ_NS_END(Atrc)
