#include <Atrc/Renderer/JitteredSubareaRenderer.h>

AGZ_NS_BEG(Atrc)

JitteredSubareaRenderer::JitteredSubareaRenderer(uint32_t spp)
    : spp_(spp)
{
    AGZ_ASSERT(spp >= 1);
}

void JitteredSubareaRenderer::Render(const Scene &scene, const Integrator &integrator, RenderTarget &rt, const SubareaRect & area) const
{
    AGZ::ObjArena<> arena;

    auto pw = rt.GetWidth(), ph = rt.GetHeight();
    Real xBaseCoef = Real(2) / pw, yBaseCoef = Real(2) / ph;
    auto cam = scene.GetCamera();
    for(uint32_t py = area.yBegin; py < area.yEnd; ++py)
    {
        Real yBase = 1 - Real(2) * py / ph;
        for(uint32_t px = area.xBegin; px < area.xEnd; ++px)
        {
            Real xBase = Real(2) * px / pw - 1;

            Spectrum pixel = SPECTRUM::BLACK;
            for(uint32_t i = 0; i < spp_; ++i)
            {
                Real xOffset = xBaseCoef * Rand();
                Real yOffset = -yBaseCoef * Rand();
                pixel += integrator.Eval(
                    scene, cam->GetRay({ xBase + xOffset, yBase + yOffset }), arena);
            }
            rt(px, py) = pixel / spp_;

            if(arena.GetUsedBytes() > 16 * 1024 * 1024)
                arena.Clear();
        }
    }
}

AGZ_NS_END(Atrc)
