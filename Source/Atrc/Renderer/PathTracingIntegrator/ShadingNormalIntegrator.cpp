#include <Atrc/Renderer/PathTracingIntegrator/ShadingNormalIntegrator.h>

AGZ_NS_BEG(Atrc)

Spectrum ShadingNormalIntegrator::Eval(const Scene &scene, const Ray &r, AGZ::ObjArena<> &arena) const
{
    SurfacePoint sp;
    if(!scene.FindCloestIntersection(r, &sp))
        return Spectrum();

    ShadingPoint shd;
    sp.entity->GetMaterial(sp)->Shade(sp, &shd, arena);

    return shd.normal.Map([](double c) { return float(c * 0.5 + 0.5); });
}


AGZ_NS_END(Atrc)
