#include <Atrc/Material/BxDF/TorranceSparrow/BlinnPhongDistribution.h>
#include <Atrc/Material/BxDF/DiffuseBRDF.h>
#include <Atrc/Material/BxDF/TorranceSparrow.h>
#include <Atrc/Material/BxDFAggregate.h>
#include <Atrc/Material/Plastic.h>

AGZ_NS_BEG(Atrc)

Plastic::Plastic(const Spectrum &kd, const Spectrum &ks, Real roughness)
    : kd_(kd / float(PI)), fresnel_(1.5f, 1.0f), md_(1 / roughness),
      torranceSparrow_(ks, &md_, &fresnel_)
{

}

void Plastic::Shade(const SurfacePoint &sp, ShadingPoint *dst, AGZ::ObjArena<> &arena) const
{
    AGZ_ASSERT(dst);

    auto *bsdf = arena.Create<BxDFAggregate>(sp.geoLocal.ez, sp.geoLocal);
    bsdf->AddBxDF(arena.Create<DiffuseBRDF>(kd_));
    bsdf->AddBxDF(&torranceSparrow_);

    dst->normal = sp.geoLocal.ez;
    dst->bsdf = bsdf;
}

AGZ_NS_END(Atrc)
