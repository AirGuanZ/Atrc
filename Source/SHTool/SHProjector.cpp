#include "SHProjector.h"

using namespace Atrc;

namespace
{
    static Real (*SH_FUNCS[9])(const Vec3&) =
    {
        AGZ::Math::GetSHByLM<Real>(0, 0),
        AGZ::Math::GetSHByLM<Real>(1, -1),
        AGZ::Math::GetSHByLM<Real>(1, 0),
        AGZ::Math::GetSHByLM<Real>(1, 1),
        AGZ::Math::GetSHByLM<Real>(2, -2),
        AGZ::Math::GetSHByLM<Real>(2, -1),
        AGZ::Math::GetSHByLM<Real>(2, 0),
        AGZ::Math::GetSHByLM<Real>(2, 1),
        AGZ::Math::GetSHByLM<Real>(2, 2),
    };
}

void SHEntityProjector::Project(const Ray &r, const Scene &scene, int N, Spectrum (&output)[9], AGZ::ObjArena<> &arena) const
{
    for(auto &s : output)
        s = Spectrum();

    SurfacePoint sp;
    if(!scene.FindCloestIntersection(r, &sp))
        return;

    ShadingPoint shd;
    sp.entity->GetMaterial(sp)->Shade(sp, &shd, arena);

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
        for(int j = 0; j < 9; ++j)
            output[j] += float(SH_FUNCS[j](bsdfSample->wi)) * pfx;
    }

    for(auto &s : output)
        s /= N;
}

void SHLightProjector::Project(const Ray &r, const Scene &scene, int N, Spectrum(&output)[9], AGZ::ObjArena<> &arena) const
{
    for(auto &s : output)
        s = Spectrum();

    if(scene.GetLights().size() != 1)
        return;

    auto light = scene.GetLights().front();

    for(int i = 0; i < N; ++i)
    {
        Real u0 = Rand(), u1 = Rand();
        auto [dir, pdf] = AGZ::Math::DistributionTransform::
            UniformOnUnitSphere<Real>::Transform({ u0, u1 });

        auto le = light->NonareaLe(Ray(Vec3(0.0), dir, EPS));
        
        for(int j = 0; j < 9; ++j)
            output[j] += le * float(SH_FUNCS[j](dir) / pdf);
    }

    for(auto &s : output)
        s /= N;
}

SHSubareaRenderer::SHSubareaRenderer(int spp, int N)
    : spp_(spp), N_(N)
{
    AGZ_ASSERT(spp >= 1 && N >= 1);
}

void SHSubareaRenderer::Render(
    const Scene &scene, const SHProjector& projector,
    RenderTarget *renderTarget, const SubareaRect &area) const
{
    AGZ::ObjArena<> arena;

    auto pw = renderTarget[0].GetWidth(), ph = renderTarget[0].GetHeight();
    Real xBaseCoef = Real(2) / pw, yBaseCoef = Real(2) / ph;
    auto cam = scene.GetCamera();
    for(uint32_t py = area.yBegin; py < area.yEnd; ++py)
    {
        Real yBase = 1 - Real(2) * py / ph;
        for(uint32_t px = area.xBegin; px < area.xEnd; ++px)
        {
            Real xBase = Real(2) * px / pw - 1;

            Spectrum pixel[9];

            for(int i = 0; i < spp_; ++i)
            {
                Real xOffset = xBaseCoef * Rand();
                Real yOffset = -yBaseCoef * Rand();
                Spectrum tpixel[9];
                projector.Project(
                    cam->GetRay({ xBase + xOffset, yBase + yOffset }), scene, N_, tpixel, arena);
                for(int k = 0; k < 9; ++k)
                    pixel[k] += tpixel[k];
            }

            for(int k = 0; k < 9; ++k)
                renderTarget[k](px, py) = pixel[k] / spp_;

            if(arena.GetUsedBytes() > 16 * 1024 * 1024)
                arena.Clear();
        }
    }
}

SHRenderer::SHRenderer(int workerCount)
    : workerCount_(workerCount)
{
    
}

void SHRenderer::Render(
    const SHSubareaRenderer &subareaRenderer, const Scene &scene, const SHProjector &projector,
    RenderTarget *output, ProgressReporter *reporter) const
{
    uint32_t w = output->GetWidth();
    uint32_t h = output->GetHeight();

    std::queue<SubareaRect> tasks;
    uint32_t yStep = w >= 256 ? 1 : 512 / w;
    uint32_t y = 0;
    for(; y + yStep <= h; y += yStep)
        tasks.push({ 0, w, y, y + yStep });
    if(y < h)
        tasks.push({ 0, w, y, h });

    if(reporter)
        reporter->Begin();

    std::atomic<size_t> finishedCount = 0;
    size_t totalCount = tasks.size();

    auto func = [&](const SubareaRect &subarea, AGZ::NoSharedParam_t)
    {
        subareaRenderer.Render(scene, projector, output, subarea);
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
