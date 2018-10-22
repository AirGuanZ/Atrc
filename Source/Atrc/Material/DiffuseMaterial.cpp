#include <Atrc/Material/BxDF/DiffuseBxDF.h>
#include <Atrc/Material/BxDFAggregate.h>
#include <Atrc/Material/DiffuseMaterial.h>

AGZ_NS_BEG(Atrc)

DiffuseMaterial::DiffuseMaterial(const Spectrum &albedo)
    : color_(AGZ::Math::InvPI<float> * albedo)
{

}

void DiffuseMaterial::Shade(const SurfacePoint &sp, ShadingPoint *dst) const
{
    AGZ_ASSERT(dst);

    auto bsdf = MakeRC<BxDFAggregate>(sp.geoLocal, sp.geoLocal);
    bsdf->AddBxDF(MakeRC<DiffuseBxDF>(color_));

    dst->shdLocal = sp.geoLocal;
    dst->bsdf = std::move(bsdf);
}

AGZ_NS_END(Atrc)
