#include <Atrc/Material/BxDF/DiffuseBRDF.h>
#include <Atrc/Material/BxDFAggregate.h>
#include <Atrc/Material/DiffuseMaterial.h>

AGZ_NS_BEG(Atrc)

DiffuseMaterial::DiffuseMaterial(const Spectrum &albedo, const NormalMapper *norMap)
    : color_(AGZ::Math::InvPI<float> * albedo), norMap_(norMap)
{
    AGZ_ASSERT(norMap);
}

void DiffuseMaterial::Shade(const SurfacePoint &sp, ShadingPoint *dst, AGZ::ObjArena<> &arena) const
{
    AGZ_ASSERT(dst);

    auto lclShdNor = norMap_->GetLocalNormal(sp.usrUV);
    dst->normal = sp.geoLocal.Local2World(lclShdNor);

    auto bsdf = arena.Create<BxDFAggregate>(dst->normal, sp.geoLocal);
    bsdf->AddBxDF(arena.Create<DiffuseBRDF>(lclShdNor, color_));
    dst->bsdf = bsdf;
}

AGZ_NS_END(Atrc)
