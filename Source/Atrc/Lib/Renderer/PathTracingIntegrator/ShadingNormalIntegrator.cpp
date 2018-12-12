#include <Atrc/Lib/Renderer/PathTracingIntegrator/ShadingNormalIntegrator.h>

namespace Atrc
{

Spectrum ShadingNormalIntegrator::Eval(const Scene &scene, const Ray &r, [[maybe_unused]] Sampler *sampler, Arena &arena) const
{
    Intersection inct;
    if(!scene.FindIntersection(r, &inct))
        return Spectrum();
    ShadingPoint shd = inct.entity->GetMaterial(inct)->GetShadingPoint(inct, arena);
    return shd.coordSys.ez.Map(
        [](Real c){ return Saturate(c / 2 + Real(0.5)); });
}

} // namespace Atrc
