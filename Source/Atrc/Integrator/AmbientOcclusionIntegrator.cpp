#include <Atrc/Integrator/AmbientOcclusionIntegrator.h>

AGZ_NS_BEG(Atrc)

AmbientOcclusionIntegrator::AmbientOcclusionIntegrator(Real maxOccuT, const Spectrum &backgroundColor, const Spectrum &objectColor)
    : maxOccuT_(maxOccuT), bgdColor_(backgroundColor), objColor_(objectColor)
{
    AGZ_ASSERT(maxOccuT > 0.0);
}

Spectrum AmbientOcclusionIntegrator::Eval(const Scene &scene, const Ray &r, [[maybe_unused]] AGZ::ObjArena<> &arena) const
{
    SurfacePoint sp;
    if(!scene.FindCloestIntersection(r, &sp))
        return bgdColor_;

    Real u0 = Rand(), u1 = Rand();
    auto [dir, pdf] = AGZ::Math::DistributionTransform::ZWeightedOnUnitHemisphere<Real>::Transform({ u0, u1 });

    dir = sp.geoLocal.Local2World(dir);

    Ray newRay(sp.pos, dir, EPS, Max(EPS, maxOccuT_));
    if(scene.HasIntersection(newRay))
        return Spectrum();
    return objColor_ * Dot(dir, sp.geoLocal.ez) / pdf;
}

AGZ_NS_END(Atrc)
