#include <Atrc/Entity/Entity.h>
#include <Atrc/Integrator/PathTracerMIS.h>
#include <Atrc/Light/LightSelector.h>
#include <Atrc/Material/BxDF.h>

AGZ_NS_BEG(Atrc)

PathTracerMIS::PathTracerMIS(int maxDepth)
    : maxDepth_(maxDepth)
{
    AGZ_ASSERT(maxDepth >= 1);
}

void PathTracerMIS::SetBackground(const Spectrum &color)
{
    background_ = color;
}

Spectrum PathTracerMIS::GetRadiance(const Scene &scene, const Ray &r) const
{
    Intersection inct;
    if(!FindClosestIntersection(scene, r, &inct))
        return background_;
    return L(scene, inct, 1);
}

Spectrum PathTracerMIS::L(const Scene &scene, const Intersection &inct, int depth) const
{
    if(depth > maxDepth_)
        return SPECTRUM::BLACK;

    auto light = inct.entity->AsLight();
    Spectrum le = light ? light->Le(inct) : SPECTRUM::BLACK;

    Spectrum ls = Ls(scene, inct, depth);

    return le + ls;
}

Spectrum PathTracerMIS::Ls(const Scene &scene, const Intersection &inct, int depth) const
{
    if(depth > maxDepth_)
        return SPECTRUM::BLACK;

    auto pbxdf = inct.entity->GetBxDF(inct);
    auto bxdf = pbxdf->GetLeafBxDF();

    if(!bxdf->CanScatter())
        return bxdf->AmbientRadiance(inct);

    if(bxdf->IsSpecular())
    {
        auto bxdfSample = bxdf->Sample(inct.wr);
        if(!bxdfSample)
            return bxdf->AmbientRadiance(inct);

        Intersection newInct;
        auto newRay = Ray(inct.pos, bxdfSample->dir, 1e-5);
        if(!FindClosestIntersection(scene, newRay, &newInct))
            return bxdf->AmbientRadiance(inct);

        return bxdf->AmbientRadiance(inct) +
               bxdfSample->coef * L(scene, newInct, depth + 1)
             * SS(Abs(Dot(inct.wr, inct.nor)) / bxdfSample->pdf);
    }

    Spectrum e = E(scene, inct, *bxdf);
    Spectrum s = S(scene, inct, *bxdf, depth);

    return e + s;
}

Spectrum PathTracerMIS::E(const Scene &scene, const Intersection &inct, const BxDF &bxdf) const
{
    constexpr Real C1 = 0.5, C2 = 1.0 - C1;

    if(Rand() < C1)
        return 1 / SS(C1) * E1(scene, inct, bxdf);

    return 1 / SS(C2) * E2(scene, inct, bxdf);
}

Spectrum PathTracerMIS::E1(const Scene &scene, const Intersection &inct, const BxDF &bxdf) const
{
    auto bxdfSample = bxdf.Sample(inct.wr);
    if(!bxdfSample)
        return SPECTRUM::BLACK;

    Intersection newInct;
    Ray newRay(inct.pos, bxdfSample->dir, 1e-5);
    if(!FindClosestIntersection(scene, newRay, &newInct))
        return SPECTRUM::BLACK;

    auto light = newInct.entity->AsLight();
    if(!light)
        return SPECTRUM::BLACK;

    auto w = 1 / SS(bxdfSample->pdf + scene.lightMgr->PDF(light) * light->SampleToPDF(inct, newInct.pos));

    return w * bxdfSample->coef * light->Le(newInct) * SS(Abs(Dot(inct.nor, bxdfSample->dir)));
}

Spectrum PathTracerMIS::E2(const Scene &scene, const Intersection &inct, const BxDF &bxdf) const
{
    if(!scene.lightMgr)
        return SPECTRUM::BLACK;

    auto lightSample = scene.lightMgr->Sample(inct);
    if(!lightSample.light)
        return SPECTRUM::BLACK;

    auto lightPnt = lightSample.light->SampleTo(inct);
    if(!lightPnt)
        return SPECTRUM::BLACK;

    lightPnt->pos += 1e-5 * lightPnt->nor;
    Vec3r dstPos = inct.pos + 1e-5 * inct.nor;

    Vec3r dir = dstPos - lightPnt->pos;
    Real dis = dir.Length();

    if(lightSample.light == inct.entity->AsLight() && dis <= 3e-5)
        return SPECTRUM::BLACK;

    dir /= dis;
    Ray shadowRay = Ray(lightPnt->pos, dir, 0, dis);
    if(((Dot(-dir, inct.nor) <= 0.0) ^ (Dot(inct.wr, inct.nor) <= 0.0))
       || HasIntersection(scene, shadowRay))
        return SPECTRUM::BLACK;

    Real lpdf = lightSample.pdf * lightPnt->pdf;
    auto w = 1 / SS(lpdf + bxdf.PDF(inct.wr, -dir));

    return bxdf.Eval(-dir, inct.wr) * lightPnt->radiance
        * SS(w * Abs(Dot(lightPnt->nor, dir) * Dot(inct.nor, -dir))
            / (dis * dis));
}

Spectrum PathTracerMIS::S(const Scene &scene, const Intersection &inct, const BxDF &bxdf, int depth) const
{
    auto bxdfSample = bxdf.Sample(inct.wr);
    if(!bxdfSample)
        return bxdf.AmbientRadiance(inct);

    auto newRay = Ray(inct.pos, bxdfSample->dir, 1e-5);
    AGZ_ASSERT(ValidDir(newRay));

    Intersection newInct;
    if(!FindClosestIntersection(scene, newRay, &newInct))
        return bxdf.AmbientRadiance(inct);

    Spectrum ret = bxdfSample->coef * Ls(scene, newInct, depth + 1)
        * SS(Abs(Dot(inct.nor, bxdfSample->dir))
            / bxdfSample->pdf)
        + bxdf.AmbientRadiance(inct);
    return ret;
}

AGZ_NS_END(Atrc)
