#include <Atrc/Lib/Camera/PinholeCamera.h>
#include <Atrc/Lib/Entity/GeometricEntity.h>
#include <Atrc/Lib/FilmFilter/BoxFilter.h>
#include <Atrc/Lib/Geometry/Sphere.h>
#include <Atrc/Lib/Geometry/TriangleBVH.h>
#include <Atrc/Lib/Light/SkyLight.h>
#include <Atrc/Lib/Material/IdealDiffuse.h>
#include <Atrc/Lib/Renderer/PathTracingIntegrator/NativePathTracingIntegrator.h>
#include <Atrc/Lib/Renderer/PathTracingRenderer.h>
#include <Atrc/Lib/Sampler/NativeSampler.h>
#include <Atrc/Lib/Texture/ConstantTexture.h>

using namespace Atrc;

int main()
{
    Sphere ground(Transform::Translate({ 0.0, 0.0, -201.0 }), 200.0);
    Sphere sphere(Transform(), 1.0);

    DefaultNormalMapper normalMapper;
    ConstantTexture albedoMap(Spectrum(Real(0.9)));

    IdealDiffuse mat(&albedoMap, &normalMapper);

    GeometricEntity groundEntity(&ground, &mat);
    GeometricEntity sphereEntity(&sphere, &mat);

    const Entity *entities[] =
    {
        &groundEntity, &sphereEntity
    };

    SkyLight sky(Spectrum(1.0));

    const Light *lights[] =
    {
        &sky
    };

    PinholeCamera camera(
        640, 480, { 2.0, 1.5 },
        1.0, Vec3(-7, 0, 0), Vec3(0.0), Vec3(0, 0, 1));

    Scene scene(entities, 2, lights, 1, &camera);
    sky.PreprocessScene(scene);

    NativePathTracingIntegrator integrator(10, 50, 1.0);
    PathTracingRenderer renderer(6, 32, integrator);

    BoxFilter filter(Vec2(0.5));
    Film film({ 640, 480 }, filter);
    NativeSampler sampler(42, 500);
    renderer.Render(scene, &sampler, &film);

    AGZ::TextureFile::WriteRGBToPNG("./Output.png", film.GetImage().Map(
        [](const Spectrum &c)
        {
            return c.Map(
                [](Real s)
                {
                    return uint8_t(Clamp(int(Saturate(s) * 256), 0, 255));
                }
            );
        }
    ));
}
