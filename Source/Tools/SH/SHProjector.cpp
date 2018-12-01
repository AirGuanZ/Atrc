#include "SHProjector.h"

using namespace Atrc;

void SHEntityProjector::Project(const Ray &r, const Scene &scene, int SHC, int N, Spectrum *output, AGZ::ObjArena<> &arena)
{
    for(int i = 0; i < SHC; ++i)
        output[i] = Spectrum();

    SurfacePoint sp;
    if(!scene.FindCloestIntersection(r, &sp))
        return;

    ShadingPoint shd;
    sp.entity->GetMaterial(sp)->Shade(sp, &shd, arena);

    auto SHTable = AGZ::Math::GetSHTable<Real>();

    for(int i = 0; i < N; ++i)
    {
        auto bsdfSample = shd.bsdf->SampleWi(sp.wo, BXDF_ALL);
        if(!bsdfSample)
            continue;

        if(scene.HasIntersection(Ray(sp.pos, bsdfSample->wi, EPS)))
            continue;

        auto f    = bsdfSample->coef;
        auto cosV = Dot(sp.geoLocal.ez, bsdfSample->wi);
        auto pS2  = bsdfSample->pdf;

        auto pfx = f * float(cosV / pS2);
        for(int j = 0; j < SHC; ++j)
            output[j] += float(SHTable[j](bsdfSample->wi)) * pfx;
    }

    for(int i = 0; i < SHC; ++i)
        output[i] /= N;
}

void SHLightProjector::Project(const Light *light, int SHC, int N, Spectrum *output)
{
    auto SHTable = AGZ::Math::GetSHTable<Real>();

    for(int i = 0; i < N; ++i)
    {
        Real u0 = Rand(), u1 = Rand();
        auto [dir, pdf] = AGZ::Math::DistributionTransform::
            UniformOnUnitSphere<Real>::Transform({ u0, u1 });

        auto le = light->NonareaLe(Ray(Vec3(0.0), dir, EPS));
        
        for(int j = 0; j < SHC; ++j)
            output[j] += le * float(SHTable[j](dir) / pdf);
    }

    for(int i = 0; i < SHC; ++i)
        output[i] /= N;
}

SHEntitySubareaRenderer::SHEntitySubareaRenderer(int spp, int SHC, int N)
    : spp_(spp), SHC_(SHC), N_(N)
{
    AGZ_ASSERT(spp >= 1 && N >= 1);
}

void SHEntitySubareaRenderer::Render(
    const Scene &scene,
    RenderTarget *renderTarget, const SubareaRect &area) const
{
    AGZ::ObjArena<> arena;

    auto pw = renderTarget[0].GetWidth(), ph = renderTarget[0].GetHeight();
    Real xBaseCoef = Real(2) / pw, yBaseCoef = Real(2) / ph;
    auto cam = scene.GetCamera();

    std::vector<Spectrum> tPixel(SHC_);

    for(uint32_t py = area.yBegin; py < area.yEnd; ++py)
    {
        Real yBase = 1 - Real(2) * py / ph;
        for(uint32_t px = area.xBegin; px < area.xEnd; ++px)
        {
            Real xBase = Real(2) * px / pw - 1;

            for(int k = 0; k < SHC_; ++k)
                renderTarget[k](px, py) = Spectrum();

            for(int i = 0; i < spp_; ++i)
            {
                Real xOffset = xBaseCoef * Rand();
                Real yOffset = -yBaseCoef * Rand();

                SHEntityProjector::Project(
                    cam->GetRay({ xBase + xOffset, yBase + yOffset }), scene, SHC_, N_, tPixel.data(), arena);

                for(int k = 0; k < SHC_; ++k)
                    renderTarget[k](px, py) += tPixel[k];
            }

            for(int k = 0; k < SHC_; ++k)
                renderTarget[k](px, py) /= spp_;

            if(arena.GetUsedBytes() > 16 * 1024 * 1024)
                arena.Clear();
        }
    }
}

SHEntityRenderer::SHEntityRenderer(int workerCount)
    : workerCount_(workerCount)
{
    
}

void SHEntityRenderer::Render(
    const SHEntitySubareaRenderer &subareaRenderer, const Scene &scene,
    RenderTarget *output, ProgressReporter *reporter) const
{
    auto tasks = GridDivider<uint32_t>::Divide(
        { 0, output->GetWidth(), 0, output->GetHeight() }, 32, 32);

    if(reporter)
        reporter->Begin();

    std::atomic<size_t> finishedCount = 0;
    size_t totalCount = tasks.size();

    auto func = [&](const SubareaRect &subarea, AGZ::NoSharedParam_t)
    {
        subareaRenderer.Render(scene, output, subarea);
        auto percent = 100.0 * ++finishedCount / totalCount;
        reporter->Report(percent);
    };

    AGZ::StaticTaskDispatcher<SubareaRect, AGZ::NoSharedParam_t> dispatcher(workerCount_);

    if(dispatcher.Run(func, AGZ::NO_SHARED_PARAM, tasks))
    {
        if(reporter)
            reporter->End();
    }
    else if(reporter)
    {
        for(auto &err : dispatcher.GetExceptions())
            reporter->Message(err.what());
    }
}
