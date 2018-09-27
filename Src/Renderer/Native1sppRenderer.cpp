#include "Native1sppRenderer.h"

AGZ_NS_BEG(Atrc)

void Native1sppRenderer::Render(
    const Scene &scene, const Integrator &integrator,
    RenderTarget<Color3f> &target) const
{
    AGZ_ASSERT(target.IsAvailable());

    const Camera *camera = scene.GetCamera();
    uint32_t pixelW = target.GetWidth(), pixelH = target.GetHeight();

    for(uint32_t pixelY = 0; pixelY != pixelH; ++pixelY)
    {
        Real screenY = 2.0 * (1.0 - (pixelY + 0.5) / pixelH) - 1.0;
        for(uint32_t pixelX = 0; pixelX != pixelW; ++pixelX)
        {
            Real screenX = 2.0 * (pixelX + 0.5) / pixelW - 1.0;
            target(pixelX, pixelY) = integrator.GetRadiance(
                scene, camera->Generate({ screenX, screenY }));
        }
    }
}

AGZ_NS_END(Atrc)
