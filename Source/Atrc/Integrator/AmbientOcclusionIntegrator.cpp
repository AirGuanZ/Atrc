#include <Atrc/Integrator/AmbientOcclusion.h>

AGZ_NS_BEG(Atrc)

AmbientOcclusionIntegrator::AmbientOcclusionIntegrator(Real maxOccuT, const Spectrum &backgroundColor, const Spectrum &objectColor)
    : maxOccuT_(maxOccuT), bgdColor_(backgroundColor), objColor_(objectColor)
{
    AGZ_ASSERT(maxOccuT > 0.0);
}

Spectrum AmbientOcclusionIntegrator::GetRadiance(const Scene &scene, const Ray &r, AGZ::ObjArena<> &arena) const
{
    SurfacePoint sp;
    if(!scene.FindCloestIntersection(r, &sp))
        return bgdColor_;

    Real u0 = Rand(), u1 = Rand();
    auto [dir, pdf] = AGZ::Math::DistributionTransform::ZWeightedOnUnitHemisphere<Real>::Transform({ u0, u1 });

    dir = sp.geoLocal.Local2World(dir);

    SurfacePoint sp2;
    if(scene.FindCloestIntersection(Ray(sp.pos, dir, EPS), &sp2) && sp2.t <= maxOccuT_)
        return Spectrum();
    return objColor_ * Dot(dir, sp.geoLocal.ez) / pdf;
}

AGZ_NS_END(Atrc)
