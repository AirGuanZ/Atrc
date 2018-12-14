#include <iostream>

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

#include <Atrc/Mgr/BuiltinCreatorRegister.h>
#include <Atrc/Mgr/Parser.h>
#include <Atrc/Mgr/Context.h>

using namespace Atrc;

int Run()
{
    AGZ::Config config;
    if(!config.LoadFromFile("./Build/scene.txt"))
        throw Mgr::MgrErr("Failed to load configuration file");
    auto &root = config.Root();

    Mgr::Context context(root);
    Mgr::RegisterBuiltinCreators(context);

    auto camera   = context.Create<Camera>(root["camera"]);
    auto renderer = context.Create<Renderer>(root["renderer"]);
    auto sampler  = context.Create<Sampler>(root["sampler"]);
    auto filter   = context.Create<FilmFilter>(root["film.filter"]);

    auto outputFilename = root["outputFilename"].AsValue();

    std::vector<Entity*> entities;
    std::vector<Light*> lights;

    auto &entArr = root["entities"].AsArray();
    for(size_t i = 0; i < entArr.Size(); ++i)
        entities.push_back(context.Create<Entity>(entArr[i]));
    
    auto &lghtArr = root["lights"].AsArray();
    for(size_t i = 0; i < lghtArr.Size(); ++i)
        lights.push_back(context.Create<Light>(lghtArr[i]));

    auto filmSize = Mgr::Parser::ParseVec2i(root["film.size"]);
    if(filmSize.x <= 0 || filmSize.y <= 0)
        throw Mgr::MgrErr("Invalid film size value");

    Film film(filmSize, *filter);

    std::vector<const Entity*> cEntities;
    std::vector<const Light*> cLights;

    cEntities.reserve(entities.size());
    for(auto ent : entities)
        cEntities.push_back(ent);

    cLights.reserve(lights.size());
    for(auto lht : lights)
        cLights.push_back(lht);
    
    Scene scene(cEntities.data(), cEntities.size(), cLights.data(), cLights.size(), camera);

    for(auto lht : lights)
        lht->PreprocessScene(scene);
    
    renderer->Render(scene, sampler, &film);

    /*Str8 outputFilename = "./Output.png";

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
    PathTracingRenderer renderer(-1, 32, integrator);

    BoxFilter filter(Vec2(0.5));
    Film film({ 640, 480 }, filter);
    NativeSampler sampler(42, 100);

    renderer.Render(scene, &sampler, &film);*/

    AGZ::TextureFile::WriteRGBToPNG(outputFilename, film.GetImage().Map(
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

    return 0;
}

int main()
{
    try
    {
        return Run();
    }
    catch(const Mgr::MgrErr &err)
    {
        const Mgr::MgrErr *pErr = &err;
        while(pErr)
        {
            std::cout << pErr->GetMsg().ToStdString() << std::endl;
            if(pErr->TryGetLeaf())
            {
                std::cout << pErr->TryGetLeaf()->what() << std::endl;
                break;
            }
            pErr = pErr->TryGetInterior();
        }
    }
    catch(const std::exception &err)
    {
        std::cout << err.what() << std::endl;
    }
    catch(...)
    {
        std::cout << "An unknown error occurred..." << std::endl;
    }
}
