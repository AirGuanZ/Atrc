#include <Atrc/Entity/Entity.h>
#include <Atrc/Integrator/PathTracerEx2.h>
#include <Atrc/Light/LightSelector.h>
#include <Atrc/Material/BxDF.h>

AGZ_NS_BEG(Atrc)

Spectrum PathTracerEx2::L(const Intersection &inct, const Scene &scene, int depth) const
{
    AGZ_ASSERT(inct.entity);

    const Light *light = inct.entity->AsLight();
    Spectrum Le = light ? light->Le(inct) : SPECTRUM::BLACK;

    return Le + Ls(inct, scene, depth);
}

Spectrum PathTracerEx2::Ls(const Intersection &inct, const Scene &scene, int depth) const
{
    AGZ_ASSERT(inct.entity);

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

        auto newRay = Ray(inct.pos, bxdfSample->dir, 1e-5);
        Intersection newInct;
        if(!FindClosestIntersection(scene, newRay, &newInct))
            return bxdf->AmbientRadiance(inct);

        return bxdf->AmbientRadiance(inct) +
            bxdfSample->coef * L(newInct, scene, depth + 1)
          * SS(Abs(Dot(inct.wr, inct.nor)) / bxdfSample->pdf);
    }

    return E(inct, *bxdf, scene) + S(inct, *bxdf, scene, depth);
}

Spectrum PathTracerEx2::E(const Intersection &inct, const BxDF &bxdf, const Scene &scene) const
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
            continue;

        ret += f * lightPnt->radiance
            * SS(Abs(Dot(lightPnt->nor, dir) * Dot(inct.nor, -dir))
                / (dis * dis * lightSam.pdf * lightPnt->pdf));
    }

    return ret / lightSampleCount_;
}

Spectrum PathTracerEx2::S(const Intersection &inct, const BxDF &bxdf, const Scene &scene, int depth) const
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

    Spectrum ret = bxdfSample->coef * Ls(newInct, scene, depth + 1)
        * SS(Abs(Dot(inct.nor, bxdfSample->dir))
            / bxdfSample->pdf)
        + bxdf.AmbientRadiance(inct);
    return ret;
}

PathTracerEx2::PathTracerEx2(int lightSampleCount, int maxDepth)
    : lightSampleCount_(lightSampleCount), maxDepth_(maxDepth), background_(SPECTRUM::BLACK)
{
    AGZ_ASSERT(lightSampleCount >= 0 && maxDepth >= 1);
}

void PathTracerEx2::SetBackground(const Spectrum &color)
{
    background_ = color;
}

Spectrum PathTracerEx2::GetRadiance(const Scene &scene, const Ray &r) const
{
    Intersection inct;
    if(!FindClosestIntersection(scene, r, &inct))
        return background_;
    return L(inct, scene, 1);
}

AGZ_NS_END(Atrc)
