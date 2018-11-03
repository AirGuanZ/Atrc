#include <Atrc/Integrator/PathTracer.h>

AGZ_NS_BEG(Atrc)

PathTracer::PathTracer(int maxDepth, const Spectrum &background)
    : maxDepth_(maxDepth), background_(background)
{
    AGZ_ASSERT(maxDepth >= 1);
}

void PathTracer::SetBackground(const Spectrum &background)
{
    background_ = background;
}

Spectrum PathTracer::GetRadiance(const Scene &scene, const Ray &r, AGZ::ObjArena<> &arena) const
{
    SurfacePoint sp;
    if(!scene.FindCloestIntersection(r, &sp))
    {
        Spectrum ret;
        for(auto light : scene.GetLights())
            ret += light->NonareaLe(r);
        return ret;
    }
    return L(scene, sp, 1, arena);
}

Spectrum PathTracer::L(const Scene &scene, const SurfacePoint &sp, int depth, AGZ::ObjArena<> &arena) const
{
    if(depth > maxDepth_)
        return Spectrum();
    
    auto light = sp.entity->AsLight();
    Spectrum le = light ? light->AreaLe(sp) : Spectrum();

    return le + Ls(scene, sp, depth, arena);
}

Spectrum PathTracer::Ls(const Scene &scene, const SurfacePoint &sp, int depth, AGZ::ObjArena<> &arena) const
{
    if(depth > maxDepth_)
        return Spectrum();

    ShadingPoint shd;
    sp.entity->GetMaterial(sp)->Shade(sp, &shd, arena);
    auto e = E(scene, sp, shd);
    auto s = S(scene, sp, shd, depth, arena);
    return e + s;
}

Spectrum PathTracer::E(const Scene &scene, const SurfacePoint &sp, const ShadingPoint &shd) const
{
    return E1(scene, sp, shd) + E2(scene, sp, shd);
}

Spectrum PathTracer::E1(const Scene &scene, const SurfacePoint &sp, const ShadingPoint &shd) const
{
    auto bsdfSample = shd.bsdf->SampleWi(sp.wo, BXDF_ALL);
    if(!bsdfSample || !bsdfSample->coef)
        return Spectrum();

    SurfacePoint newInct;
    Ray newRay(sp.pos, bsdfSample->wi, EPS);
    if(scene.FindCloestIntersection(newRay, &newInct))
    {
        auto light = newInct.entity->AsLight();
        if(!light)
            return Spectrum();

        if(bsdfSample->type & BXDF_SPECULAR)
        {
            return bsdfSample->coef * Abs(Dot(shd.shdLocal.ez, bsdfSample->wi))
                 * light->AreaLe(newInct) / bsdfSample->pdf;
        }

        Real lpdf = scene.SampleLightPDF(light) * light->SampleLiPDF(newInct.pos, sp);
        return bsdfSample->coef * Abs(Dot(shd.shdLocal.ez, bsdfSample->wi))
             * light->AreaLe(newInct) / (bsdfSample->pdf + lpdf);
    }

    auto [light, lpdf] = scene.SampleLight();

    // delta光源无法被BSDF采样命中
    if(!light || light->IsDelta())
        return Spectrum();

    auto le = light->NonareaLe(newRay);
    if(!le)
        return Spectrum();

    // Specular BSDF的coef和pdf都是delta
    if(bsdfSample->type & BXDF_SPECULAR)
        return bsdfSample->coef * Abs(Dot(shd.shdLocal.ez, bsdfSample->wi))
        * le / bsdfSample->pdf;

    lpdf *= light->SampleLiPDF(sp.pos + bsdfSample->wi, sp, false);
    return bsdfSample->coef * Abs(Dot(shd.shdLocal.ez, bsdfSample->wi))
         * le / (bsdfSample->pdf + lpdf);
}

Spectrum PathTracer::E2(const Scene &scene, const SurfacePoint &sp, const ShadingPoint &shd) const
{
    auto [light, lpdf] = scene.SampleLight();
    if(!light)
        return Spectrum();

    auto lightSample = light->SampleLi(sp);
    if(!lightSample.radiance)
        return Spectrum();

    Spectrum f = shd.bsdf->Eval(lightSample.wi, sp.wo, BXDF_ALL);
    if(!f)
        return Spectrum();

    // Shadow ray测试
    Ray shadowRay(sp.pos, lightSample.wi, EPS, (lightSample.pos - sp.pos).Length() - EPS);
    if(scene.HasIntersection(shadowRay) ||
        ((Dot(sp.wo,          sp.geoLocal.ez) <= 0.0) ^
         (Dot(lightSample.wi, sp.geoLocal.ez) <= 0.0)))
        return Spectrum();

    lpdf *= lightSample.pdf;

    // Delta light的pdf是delta
    if(light->IsDelta())
    {
        return f * Abs(Dot(sp.geoLocal.ez, lightSample.wi))
             * lightSample.radiance / lpdf;
    }

    Real bpdf = shd.bsdf->SampleWiPDF(lightSample.wi, sp.wo, BxDFType(BXDF_ALL & ~BXDF_SPECULAR));
    return f * Abs(Dot(sp.geoLocal.ez, lightSample.wi))
         * lightSample.radiance / (bpdf + lpdf);
}

Spectrum PathTracer::S(const Scene &scene, const SurfacePoint &sp, const ShadingPoint &shd, int depth, AGZ::ObjArena<> &arena) const
{
    auto bsdfSample = shd.bsdf->SampleWi(sp.wo, BXDF_ALL);
    if(!bsdfSample || !bsdfSample->coef)
        return Spectrum();

    SurfacePoint newInct;
    Ray newRay(sp.pos, bsdfSample->wi, EPS);
    if(!scene.FindCloestIntersection(newRay, &newInct))
        return Spectrum();

    return bsdfSample->coef * Abs(Dot(shd.shdLocal.ez, bsdfSample->wi))
         * Ls(scene, newInct, depth + 1, arena) / bsdfSample->pdf;
}

AGZ_NS_END(Atrc)
