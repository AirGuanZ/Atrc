#include <Atrc/Lib/Core/Ray.h>
#include <Atrc/Lib/Core/Sampler.h>
#include <Atrc/Lib/Core/Scene.h>
#include <Atrc/Lib/Renderer/PathTracingIntegrator/NativePathTracingIntegrator.h>

namespace Atrc
{

NativePathTracingIntegrator::NativePathTracingIntegrator(
    int minDepth, int maxDepth, Real contProb) noexcept
    : minDepth_(minDepth), maxDepth_(maxDepth), contProb_(contProb)
{
    AGZ_ASSERT(0 < minDepth && minDepth <= maxDepth);
    AGZ_ASSERT(0 < contProb && contProb <= 1);
}

Spectrum NativePathTracingIntegrator::Eval(
    const Scene &scene, const Ray &r, Sampler *sampler, Arena &arena) const
{
    Spectrum coef = Spectrum(Real(1)), ret;
    Ray ray = r;

    for(int i = 1; i <= maxDepth_; ++i)
    {
        // Russian roulette strategy

        if(i > minDepth_)
        {
            if(sampler->GetReal() > contProb_)
                break;
            coef /= contProb_;
        }

        // Find closest intersection

        Intersection inct;
        if(!scene.FindIntersection(ray, &inct))
        {
            Spectrum le;
            for(auto light : scene.GetLights())
                le += light->NonAreaLe(ray);
            ret += coef * le;
            break;
        }

        // Compute le

        auto light = inct.entity->AsLight();
        if(light)
            ret += coef * light->AreaLe(inct);
        
        // Sample BSDF

        ShadingPoint shd = inct.material->GetShadingPoint(inct, arena);
        auto bsdfSample = shd.bsdf->SampleWi(
            shd.coordSys, inct.coordSys, inct.wr, BSDF_ALL, false, sampler->GetReal2());
        if(!bsdfSample || !bsdfSample->coef)
            break;
        
        // Update coef and construct the next ray

        coef *= bsdfSample->coef * Abs(Cos(bsdfSample->wi, shd.coordSys.ez)) / bsdfSample->pdf;
        ray = Ray(inct.pos, bsdfSample->wi, EPS);
    }

    return ret;
}

} // namespace Atrc
