#include "Native1sppRenderer.h"

AGZ_NS_BEG(Atrc)

void Native1sppSubareaRenderer::Render(
    const Scene &scene, const Integrator &integrator,
    RenderTarget<Color3f> &target, const RenderTargetSubarea &area) const
{
    AGZ_ASSERT(target.IsAvailable());

    const Camera *camera = scene.GetCamera();
    uint32_t pixelW = target.GetWidth(), pixelH = target.GetHeight();

    for(uint32_t pixelY = area.yBegin; pixelY != area.yEnd; ++pixelY)
    {
        Real screenY = 2.0 * (1.0 - (pixelY + 0.5) / pixelH) - 1.0;
        for(uint32_t pixelX = area.xBegin; pixelX != area.xEnd; ++pixelX)
        {
            Real screenX = 2.0 * (pixelX + 0.5) / pixelW - 1.0;
            target(pixelX, pixelY) = integrator.GetRadiance(
                scene, camera->Generate({ screenX, screenY }));
        }
    }
}

AGZ_NS_END(Atrc)
