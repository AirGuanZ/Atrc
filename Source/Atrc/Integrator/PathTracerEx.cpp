#include <Atrc/Entity/Entity.h>
#include <Atrc/Integrator/PathTracerEx.h>
#include <Atrc/Light/LightSelector.h>
#include <Atrc/Material/BxDF.h>

AGZ_NS_BEG(Atrc)

Spectrum PathTracerEx::L(const Ray &r, const Intersection &inct, const Scene &scene, int depth) const
{
    AGZ_ASSERT(inct.entity);

    const Light *light = inct.entity->AsLight();
    Spectrum Le = light ? light->Le(inct) : SPECTRUM::BLACK;

    auto bxdf = inct.entity->GetBxDF(inct);
    if(!bxdf->CanScatter())
        return Le + bxdf->AmbientRadiance(inct);

    Spectrum Es = E(r, inct, *bxdf, scene);
    Spectrum Ss = S(r, inct, *bxdf, scene, depth);

    return Le + Es + Ss;
}

Spectrum PathTracerEx::E(const Ray &r, const Intersection &inct, const BxDF &bxdf, const Scene &scene) const
{
    Spectrum ret = SPECTRUM::BLACK;
    if(!scene.lightMgr || !lightSampleCount_)
        return ret;

    for(int i = 0; i < lightSampleCount_; ++i)
    {
        auto lightSam = scene.lightMgr->Sample(inct);
        if(!lightSam.light)
            continue;

        auto lightPnt = lightSam.light->SampleTo(inct);
        if(!lightPnt)
            continue;

        lightPnt->pos += 1e-5 * lightPnt->nor;
        Vec3r dstPos = inct.pos + 1e-5 * inct.nor;

        Vec3r dir = dstPos - lightPnt->pos;
        Real dis = dir.Length();

        if(lightSam.light == inct.entity->AsLight() && dis <= 3e-5)
            continue;

        dir /= dis;
        Ray shadowRay = Ray(lightPnt->pos, dir, 0, dis);
        if(((Dot(-dir, inct.nor) <= 0.0) ^ (Dot(inct.wr, inct.nor) <= 0.0))
          || HasIntersection(scene, shadowRay))
            continue;

        auto f = bxdf.Eval(-dir, inct.wr);
        if(f == SPECTRUM::BLACK)
            return SPECTRUM::BLACK;

        ret += f * lightPnt->radiance
             * SS(Abs(Dot(lightPnt->nor, dir) * Dot(inct.nor, -dir))
                / (dis * dis * lightSam.pdf * lightPnt->pdf));
    }

    return ret / lightSampleCount_;
}

Spectrum PathTracerEx::S(const Ray &r, const Intersection &inct, const BxDF &bxdf, const Scene &scene, int depth) const
{
    if(depth > maxDepth_)
        return SPECTRUM::BLACK;

    auto bxdfSample = bxdf.Sample(inct.wr);
    if(!bxdfSample)
        return bxdf.AmbientRadiance(inct);

    auto newRay = Ray(inct.pos, bxdfSample->dir, 1e-5);
    AGZ_ASSERT(ValidDir(newRay));

    Intersection newInct;
    if(!FindClosestIntersection(scene, newRay, &newInct))
        return bxdf.AmbientRadiance(inct);
    auto newBxDF = newInct.entity->GetBxDF(newInct);

    Spectrum EpS = SPECTRUM::BLACK;
    if(newBxDF->CanScatter())
    {
        EpS += E(newRay, newInct, *newBxDF, scene);
        EpS += S(newRay, newInct, *newBxDF, scene, depth + 1);
    }
    else
        EpS = newBxDF->AmbientRadiance(newInct);

    Spectrum ret = bxdfSample->coef * EpS
                 * SS(Abs(Dot(inct.nor, bxdfSample->dir))
                    / bxdfSample->pdf)
                 + bxdf.AmbientRadiance(inct);
    return ret;
}

PathTracerEx::PathTracerEx(int lightSampleCount, int maxDepth)
    : lightSampleCount_(lightSampleCount), maxDepth_(maxDepth)
{
    AGZ_ASSERT(lightSampleCount >= 0 && maxDepth >= 1);
}

void PathTracerEx::SetBackground(const Spectrum &color)
{
    background_ = color;
}

Spectrum PathTracerEx::GetRadiance(const Scene &scene, const Ray &r) const
{
    Intersection inct;
    if(!FindClosestIntersection(scene, r, &inct))
        return background_;
    return L(r, inct, scene, 1);
}

AGZ_NS_END(Atrc)
