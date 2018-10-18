#include <Atrc/Core/Material.h>

AGZ_NS_BEG(Atrc)

void Material::ComputeShadingLocal(SurfacePoint *sp) const
{
    AGZ_ASSERT(sp && !sp->shdLocal);
    sp->shdLocal = Some(sp->geoLocal);
}

AGZ_NS_END(Atrc)
