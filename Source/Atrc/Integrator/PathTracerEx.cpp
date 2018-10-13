#include <Atrc/Entity/Entity.h>
#include <Atrc/Integrator/PathTracerEx.h>
#include <Atrc/Light/LightManager.h>
#include <Atrc/Material/BxDF.h>

AGZ_NS_BEG(Atrc)

Spectrum PathTracerEx::L(const Ray &r, const Intersection &inct, const Scene &scene, int depth) const
{
    AGZ_ASSERT(inct.entity);

    const Light *light = inct.entity->AsLight();

    auto bxdf = inct.entity->GetBxDF(inct);

    Spectrum Le = light ? light->Le(inct) : SPECTRUM::BLACK;
    Spectrum Es = E(r, inct, *bxdf, scene, depth);
    Spectrum Ss = S(r, inct, *bxdf, scene, depth);

    return Le + Es + Ss;
}

Spectrum PathTracerEx::E(const Ray &r, const Intersection &inct, const BxDF &bxdf, const Scene &scene, int depth) const
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

        lightPnt->pos += 1e-4 * lightPnt->nor;
        Vec3r dir = inct.pos - lightPnt->pos;
        Real dis = dir.Length();

        if(lightSam.light == inct.entity->AsLight() && dis <= 3e-5)
            continue;

        dir /= dis;
        Ray shadowRay = Ray(lightPnt->pos, dir / dis, 1e-5, dis - 1e-5);
        if(HasIntersection(scene, shadowRay))
            continue;

        ret += bxdf.Eval(-dir, inct.wr) * lightPnt->radiance
             * SS(Abs(Dot(lightPnt->nor, dir) * Dot(inct.nor, -dir))
                / (dis * dis * lightSam.pdf * lightPnt->pdf));
    }

    return ret / lightSampleCount_;
}

Spectrum PathTracerEx::S(const Ray &r, const Intersection &inct, const BxDF &bxdf, const Scene &scene, int depth) const
{
    SS RRCoef = 1.0f;
    if(depth >= minDepth_)
    {
        if(depth > maxDepth_ || Rand() > contProb_)
            return SPECTRUM::BLACK;
        RRCoef = 1 / SS(contProb_);
    }

    auto bxdfSample = bxdf.Sample(inct.wr, BXDF_ALL);
    if(!bxdfSample)
        return RRCoef * bxdf.AmbientRadiance(inct);

    auto newRay = Ray(inct.pos, bxdfSample->dir, 1e-5);
    AGZ_ASSERT(ValidDir(newRay));

    Intersection newInct;
    if(!FindClosestIntersection(scene, newRay, &newInct))
        return SPECTRUM::BLACK;
    auto newBxDF = newInct.entity->GetBxDF(newInct);

    Spectrum EpS = E(newRay, newInct, *newBxDF, scene, depth + 1);
    EpS += S(newRay, newInct, *newBxDF, scene, depth + 1);

    Spectrum ret = bxdfSample->coef * EpS
                 * SS(Abs(Dot(inct.nor, bxdfSample->dir))
                    / bxdfSample->pdf)
                 + bxdf.AmbientRadiance(inct);
    return RRCoef * ret;
}

PathTracerEx::PathTracerEx(int lightSampleCount, Real contProb, int minDepth, int maxDepth)
    : lightSampleCount_(lightSampleCount), contProb_(contProb), minDepth_(minDepth), maxDepth_(maxDepth)
{
    AGZ_ASSERT(lightSampleCount >= 0 && 0.0 <= contProb && contProb <= 1.0);
    AGZ_ASSERT(1 <= minDepth && minDepth <= maxDepth);
}

Spectrum PathTracerEx::GetRadiance(const Scene &scene, const Ray &r) const
{
    Intersection inct;
    if(!FindClosestIntersection(scene, r, &inct))
        return SPECTRUM::BLACK;
    return L(r, inct, scene, 1);
}

AGZ_NS_END(Atrc)
