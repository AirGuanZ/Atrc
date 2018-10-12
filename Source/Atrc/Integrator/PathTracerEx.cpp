#include <Atrc/Entity/Entity.h>
#include <Atrc/Integrator/PathTracerEx.h>
#include <Atrc/Material/BxDF.h>

AGZ_NS_BEG(Atrc)

Spectrum PathTracerEx::L(const Ray &r, const Intersection &inct, const Scene &scene, int depth) const
{
    AGZ_ASSERT(inct.entity);
    auto bxdf = inct.entity->GetBxDF(inct);

    const Light *light = inct.entity->AsLight();

    Spectrum Le = light ? light->Le(inct) : SPECTRUM::BLACK;
    Spectrum Es = E(r, inct, *bxdf, scene, depth);
    Spectrum Ss = S(r, inct, *bxdf, scene, depth);

    return Le + Es + Ss;
}

Spectrum PathTracerEx::E(const Ray &r, const Intersection &inct, const BxDF &bxdf, const Scene &scene, int depth) const
{
    // TODO
    return SPECTRUM::BLACK;
}

Spectrum PathTracerEx::S(const Ray &r, const Intersection &inct, const BxDF &bxdf, const Scene &scene, int depth) const
{
    // TODO
    return SPECTRUM::BLACK;
}

PathTracerEx::PathTracerEx(Real contProb, int minDepth, int maxDepth)
    : contProb_(contProb), minDepth_(minDepth), maxDepth_(maxDepth)
{

}

Spectrum PathTracerEx::GetRadiance(const Scene &scene, const Ray &r) const
{
    Intersection inct;
    if(!FindClosestIntersection(scene, r, &inct))
        return SPECTRUM::BLACK;
    return L(r, inct, scene, 1);
}

AGZ_NS_END(Atrc)
