#include <Atrc/Integrator/PureColorIntegrator.h>

AGZ_NS_BEG(Atrc)

PureColorIntegrator::PureColorIntegrator(const Spectrum &background, const Spectrum &entity)
    : background_(background), entity_(entity)
{
    
}

Spectrum PureColorIntegrator::GetRadiance(const Scene &scene, const Ray &r, [[maybe_unused]] AGZ::ObjArena<> &arena) const
{
    return scene.HasIntersection(r) ? entity_ : background_;
}

AGZ_NS_END(Atrc)
