#include <Atrc/Lib/Camera/PinholeCamera.h>
#include <Atrc/Lib/Entity/GeometricEntity.h>
#include <Atrc/Lib/FilmFilter/BoxFilter.h>
#include <Atrc/Lib/Geometry/Sphere.h>
#include <Atrc/Lib/Material/IdealBlack.h>
#include <Atrc/Lib/Renderer/PathTracingIntegrator/ShadingNormalIntegrator.h>
#include <Atrc/Lib/Renderer/PathTracingRenderer.h>
#include <Atrc/Lib/Sampler/NativeSampler.h>

using namespace Atrc;

int main()
{
    Sphere ground(Transform::Translate({ 0.0, 0.0, -201.0 }), 200.0);
    Sphere sphere(Transform(), 1.0);

    GeometricEntity groundEntity(&ground, &STATIC_IDEAL_BLACK);
    GeometricEntity sphereEntity(&sphere, &STATIC_IDEAL_BLACK);

    const Entity *entities[] =
    {
        &groundEntity, &sphereEntity
    };

    PinholeCamera camera(
        640, 480, { 2.0, 1.5 },
        1.0, Vec3(-7, 0, 0), Vec3(0.0), Vec3(0, 0, 1));

    Scene scene(entities, 2, nullptr, 0, &camera);

    ShadingNormalIntegrator integrator;
    PathTracingRenderer renderer(-1, 32, integrator);

    BoxFilter filter(Vec2(0.5));
    Film film({ 640, 480 }, filter);
    NativeSampler sampler(42, 100);
    renderer.Render(scene, &sampler, &film);

    AGZ::TextureFile::WriteRGBToPNG("./Output.png", film.GetImage().Map(
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
