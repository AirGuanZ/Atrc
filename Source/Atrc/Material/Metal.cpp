#include <Atrc/Material/BxDF/TorranceSparrow/BlinnPhongDistribution.h>
#include <Atrc/Material/BxDF/TorranceSparrow.h>
#include <Atrc/Material/BxDFAggregate.h>
#include <Atrc/Material/Metal.h>

AGZ_NS_BEG(Atrc)

Metal::Metal(const Spectrum &rc, const Spectrum &etaI, const Spectrum &etaT, const Spectrum &k, Real roughness)
    : fresnel_(etaI, etaT, k), md_(1 / roughness), torranceSparrow_(rc, &md_, &fresnel_)
{

}

void Metal::Shade(const SurfacePoint &sp, ShadingPoint *dst, AGZ::ObjArena<> &arena) const
{
    AGZ_ASSERT(dst);

    auto *bsdf = arena.Create<BxDFAggregate>(sp.geoLocal.ez, sp.geoLocal);
    bsdf->AddBxDF(&torranceSparrow_);

    dst->normal = sp.geoLocal.ez;
    dst->bsdf = bsdf;
}

AGZ_NS_END(Atrc)
