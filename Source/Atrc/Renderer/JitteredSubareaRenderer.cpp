#include <Atrc/Camera/Camera.h>
#include <Atrc/Renderer/JitteredSubareaRenderer.h>

AGZ_NS_BEG(Atrc)

JitteredSubareaRenderer::JitteredSubareaRenderer(uint32_t spp)
    : spp_(spp)
{

}

void JitteredSubareaRenderer::Render(
    const Scene &scene, const Integrator &integrator,
    RenderTarget<Color3f> &output, const Subarea &area) const
{
    auto pw = output.GetWidth(), ph = output.GetHeight();
    Real xBaseCoef = 2.0 / pw, yBaseCoef = 2.0 / ph;
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
                    scene, scene.camera->GetRay({ xBase + xOffset, yBase + yOffset }));
            }
            output(px, py) = pixel / spp_;
        }
    }
}

AGZ_NS_END(Atrc)
