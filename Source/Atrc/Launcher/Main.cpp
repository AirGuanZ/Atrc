#include <iostream>

#include <Atrc/Lib/Camera/PinholeCamera.h>
#include <Atrc/Lib/Entity/GeometricEntity.h>
#include <Atrc/Lib/Geometry/Sphere.h>
#include <Atrc/Lib/Material/PureBlack.h>
#include <Atrc/Lib/Renderer/PathTracingIntegrator/ShadingNormalIntegrator.h>
#include <Atrc/Lib/Renderer/PathTracingRenderer.h>

using namespace Atrc;

int main()
{
    Sphere ground(Transform::Translate({ 0.0, 0.0, -201.0 }), 200.0);
    Sphere sphere(Transform(), 1.0);

    GeometricEntity groundEntity(&ground, &STATIC_PURE_BLACK);
    GeometricEntity sphereEntity(&sphere, &STATIC_PURE_BLACK);

    const Entity *entities[] =
    {
        &groundEntity, &sphereEntity
    };

    PinholeCamera camera(
        640, 480, { 2.0, 1.5 },
        1.0, Vec3(-7, 0, 0), Vec3(0.0), Vec3(0, 0, 1));

    Scene scene(entities, 2, nullptr, 0, &camera);

    ShadingNormalIntegrator integrator;
    PathTracingRenderer renderer(-1, 1000, 16, integrator);

    RenderTarget renderTarget(640, 480);
    renderer.Render(scene, &renderTarget);

    AGZ::TextureFile::WriteRGBToPNG("./Output.png", renderTarget.Map(
        [](const Spectrum &c)
        {
            return c.Map(
                [](float s)
                {
                    return uint8_t(Saturate(s) * 255);
                }
            );
        }
    ));
}
