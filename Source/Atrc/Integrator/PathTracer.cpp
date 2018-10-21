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

Spectrum PathTracer::GetRadiance(const Scene &scene, const Ray &r) const
{
    SurfacePoint sp;
    if(!scene.FindCloestIntersection(r, &sp))
    {
        Spectrum ret;
        for(auto light : scene.GetLights())
            ret += light->Le(r);
        return ret;
    }
    return L(scene, sp, 1);
}

Spectrum PathTracer::L(const Scene &scene, const SurfacePoint &sp, int depth) const
{
    if(depth > maxDepth_)
        return Spectrum();
    
    auto light = sp.entity->AsLight();
    Spectrum le = light ? light->AreaLe(sp) : Spectrum();

    return le + Ls(scene, sp, depth);
}

Spectrum PathTracer::Ls(const Scene &scene, const SurfacePoint &sp, int depth) const
{
    if(depth > maxDepth_)
        return Spectrum();

    ShadingPoint shd;
    sp.entity->GetMaterial(sp)->Shade(sp, &shd);
    auto e = E(scene, sp, shd);
    auto s = S(scene, sp, shd, depth);
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
    Ray newRay(sp.pos, bsdfSample->wi, 1e-5);
    if(scene.FindCloestIntersection(newRay, &newInct))
    {
        auto light = newInct.entity->AsLight();
        if(!light)
            return Spectrum();

        if(bsdfSample->type == BXDF_SPECULAR)
        {
            return bsdfSample->coef * Abs(Dot(shd.shdLocal.ez, bsdfSample->wi))
                 * light->AreaLe(newInct) / bsdfSample->pdf;
        }

        Real lpdf = scene.SampleLightPDF(light) * light->SampleToPDF(newInct.pos, sp.pos);
        return bsdfSample->coef * Abs(Dot(shd.shdLocal.ez, bsdfSample->wi))
             * light->AreaLe(newInct) / (bsdfSample->pdf + lpdf);
    }

    auto [light, lpdf] = scene.SampleLight();

    // delta光源无法被BSDF采样命中
    if(!light || light->IsDelta())
        return Spectrum();

    auto le = light->Le(newRay);
    if(!le)
        return Spectrum();

    // Specular BSDF的coef和pdf都是delta
    if(bsdfSample->type == BXDF_SPECULAR)
        return bsdfSample->coef * Abs(Dot(shd.shdLocal.ez, bsdfSample->wi))
        * le / bsdfSample->pdf;

    lpdf *= light->SampleToPDF(Vec3(), sp.pos, false);
    return bsdfSample->coef * Abs(Dot(shd.shdLocal.ez, bsdfSample->wi))
         * le / (bsdfSample->pdf + lpdf);
}

Spectrum PathTracer::E2(const Scene &scene, const SurfacePoint &sp, const ShadingPoint &shd) const
{
    auto [light, lpdf] = scene.SampleLight();
    if(!light)
        return Spectrum();

    auto lightSample = light->SampleTo(sp);
    if(!lightSample.radiance)
        return Spectrum();

    Spectrum f = shd.bsdf->Eval(lightSample.wi, sp.wo);
    if(!f)
        return Spectrum();

    // Shadow ray测试
    Ray shadowRay(sp.pos, lightSample.wi, 1e-5, (lightSample.pos - sp.pos).Length() - 1e-5);
    if(scene.HasIntersection(shadowRay))
        return Spectrum();

    // Delta light的pdf是delta
    if(light->IsDelta())
    {
        return f * Abs(Dot(sp.geoLocal.ez, lightSample.wi))
             * lightSample.radiance / lightSample.pdf;
    }

    Real bpdf = shd.bsdf->SampleWiPDF(lightSample.wi, sp.wo, BxDFType(BXDF_ALL & ~BXDF_SPECULAR));
    lpdf *= lightSample.pdf;
    return f * Abs(Dot(sp.geoLocal.ez, lightSample.wi))
         * lightSample.radiance / (bpdf + lpdf);
}

Spectrum PathTracer::S(const Scene &scene, const SurfacePoint &sp, const ShadingPoint &shd, int depth) const
{
    auto bsdfSample = shd.bsdf->SampleWi(sp.wo, BXDF_ALL);
    if(!bsdfSample || !bsdfSample->coef)
        return Spectrum();

    SurfacePoint newInct;
    Ray newRay(sp.pos, bsdfSample->wi, 1e-5);
    if(!scene.FindCloestIntersection(newRay, &newInct))
        return Spectrum();

    return bsdfSample->coef * Abs(Dot(shd.shdLocal.ez, bsdfSample->wi))
         * Ls(scene, newInct, depth + 1) / bsdfSample->pdf;
}

AGZ_NS_END(Atrc)
