#include <Atrc/Renderer/JitteredSubareaRenderer.h>

AGZ_NS_BEG(Atrc)

JitteredSubareaRenderer::JitteredSubareaRenderer(uint32_t spp)
    : spp_(spp)
{
    AGZ_ASSERT(spp >= 1);
}

void JitteredSubareaRenderer::Render(const Scene &scene, const Integrator &integrator, RenderTarget &rt, const SubareaRect & area) const
{
    auto pw = rt.GetWidth(), ph = rt.GetHeight();
    Real xBaseCoef = 2.0 / pw, yBaseCoef = 2.0 / ph;
    auto cam = scene.GetCamera();
    for(uint32_t py = area.yBegin; py < area.yEnd; ++py)
    {
        Real yBase = 1.0 - 2.0 * py / ph;
        for(uint32_t px = area.xBegin; px < area.xEnd; ++px)
        {
            Real xBase = 2.0 * px / pw - 1.0;

            Spectrum pixel = SPECTRUM::BLACK;
            for(uint32_t i = 0; i < spp_; ++i)
            {
                Real xOffset = xBaseCoef * Rand();
                Real yOffset = -yBaseCoef * Rand();
                pixel += integrator.GetRadiance(
                    scene, cam->GetRay({ xBase + xOffset, yBase + yOffset }));
            }
            rt(px, py) = pixel / spp_;
        }
    }
}

AGZ_NS_END(Atrc)
