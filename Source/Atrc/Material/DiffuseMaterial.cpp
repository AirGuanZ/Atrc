#include <Atrc/Material/BxDF/DiffuseBRDF.h>
#include <Atrc/Material/BxDFAggregate.h>
#include <Atrc/Material/DiffuseMaterial.h>
#include <Atrc/Utility/ParamParser.h>

AGZ_NS_BEG(Atrc)

DiffuseMaterial::DiffuseMaterial(const Spectrum &albedo)
    : color_(AGZ::Math::InvPI<float> * albedo)
{

}

void DiffuseMaterial::Shade(const SurfacePoint &sp, ShadingPoint *dst, AGZ::ObjArena<> &arena) const
{
    AGZ_ASSERT(dst);

    auto bsdf = arena.Create<BxDFAggregate>(sp.geoLocal, sp.geoLocal);
    bsdf->AddBxDF(arena.Create<DiffuseBRDF>(color_));

    dst->shdLocal = sp.geoLocal;
    dst->bsdf = bsdf;
}

AGZ_NS_END(Atrc)
