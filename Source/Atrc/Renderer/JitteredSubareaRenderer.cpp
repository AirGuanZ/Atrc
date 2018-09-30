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
    Real xBaseCoef = Real(2) / pw, yBaseCoef = Real(2) / ph;
    for(uint32_t py = area.yBegin; py < area.yEnd; ++py)
    {
        Real yBase = Real(1) - Real(2) * py / ph;
        for(uint32_t px = area.xBegin; px < area.xEnd; ++px)
        {
            Real xBase = Real(2) * px / pw - Real(1);

            Spectrum pixel = SPECTRUM::BLACK;
            for(uint32_t i = 0; i < spp_; ++i)
            {
                Real xOffset = xBaseCoef * Random::Uniform(Real(0), Real(1));
                Real yOffset = -yBaseCoef * Random::Uniform(Real(0), Real(1));
                pixel += integrator.GetRadiance(
                    scene, scene.camera->GetRay({ xBase + xOffset, yBase + yOffset }));
            }
            output(px, py) = pixel / spp_;
        }
    }
}

AGZ_NS_END(Atrc)
