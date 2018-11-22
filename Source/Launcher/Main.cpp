#include <iostream>
#include <Atrc/Atrc.h>

#include "SceneManager/EntityCreator.h"
#include "SceneManager/GeometryManager.h"
#include "SceneManager/IntegratorManager.h"
#include "SceneManager/LightManager.h"
#include "SceneManager/MaterialManager.h"
#include "SceneManager/MediumManager.h"
#include "SceneManager/PostProcessorManager.h"
#include "SceneManager/RendererManager.h"

#include "SceneManager/SceneManager.h"

#if defined(_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>
#endif

using namespace AGZ;
using namespace Atrc;
using namespace std;

Texture2D<Color3b> ToSavedImage(const RenderTarget &origin)
{
    return origin.Map([=](const Color3f &color)
    {
        return color.Map([=](float x)
        {
            return static_cast<uint8_t>(Clamp(x, 0.0f, 1.0f) * 255);
        });
    });
}

void InitializeObjectManagers()
{
    EntityCreator *ENTITY_CREATORS[] =
    {
        GeometricEntityCreator      ::GetInstancePtr(),
        GeometricDiffuseLightCreator::GetInstancePtr(),
    };
    for(auto c : ENTITY_CREATORS)
        EntityManager::GetInstance().AddCreator(c);

    GeometryCreator *GEOMETRY_CREATORS[] =
    {
        CubeCreator       ::GetInstancePtr(),
        SphereCreator     ::GetInstancePtr(),
        TriangleBVHCreator::GetInstancePtr(),
    };
    for(auto c : GEOMETRY_CREATORS)
        GeometryManager::GetInstance().AddCreator(c);

    LightCreator *LIGHT_CREATORS[] =
    {
        CubeEnvironmentLightCreator::GetInstancePtr(),
        DirectionalLightCreator    ::GetInstancePtr(),
        SkyLightCreator            ::GetInstancePtr(),
    };
    for(auto c : LIGHT_CREATORS)
        LightManager::GetInstance().AddCreator(c);

    IntegratorCreator *INTEGRATOR_CREATORS[] =
    {
        AmbientOcclusionIntegratorCreator::GetInstancePtr(),
        PathTracerCreator                ::GetInstancePtr(),
        PureColorIntegratorCreator       ::GetInstancePtr(),
        VolumetricPathTracerCreator      ::GetInstancePtr(),
    };
    for(auto c : INTEGRATOR_CREATORS)
        IntegratorManager::GetInstance().AddCreator(c);

    MaterialCreator *MATERIAL_CREATORS[] =
    {
        BlackMaterialCreator     ::GetInstancePtr(),
        DiffuseMaterialCreator   ::GetInstancePtr(),
        FresnelSpecularCreator   ::GetInstancePtr(),
        IdealMirrorCreator       ::GetInstancePtr(),
        MetalCreator             ::GetInstancePtr(),
        PlasticCreator           ::GetInstancePtr(),
        TextureScalerCreator     ::GetInstancePtr(),
        UncallableMaterialCreator::GetInstancePtr(),
    };
    for(auto c : MATERIAL_CREATORS)
        MaterialManager::GetInstance().AddCreator(c);

    MediumCreator *MEDIUM_CREATORS[] =
    {
        HomongeneousMediumCreator::GetInstancePtr(),
    };
    for(auto c : MEDIUM_CREATORS)
        MediumManager::GetInstance().AddCreator(c);

    PostProcessorStageCreator *POSTPROCESSOR_STAGE_CREATORS[] =
    {
        ACESFilmCreator      ::GetInstancePtr(),
        GammaCorrectorCreator::GetInstancePtr(),
    };
    for(auto c : POSTPROCESSOR_STAGE_CREATORS)
        PostProcessorStageManager::GetInstance().AddCreator(c);

    ProgressReporterCreator *REPORTER_CREATORS[] =
    {
        DefaultProgressReporterCreator::GetInstancePtr(),
    };
    for(auto c : REPORTER_CREATORS)
        ProgressReporterManager::GetInstance().AddCreator(c);

    RendererCreator *RENDERER_CREATORS[] =
    {
        ParallelRendererCreator::GetInstancePtr(),
        SerialRendererCreator  ::GetInstancePtr(),
    };
    for(auto c : RENDERER_CREATORS)
        RendererManager::GetInstance().AddCreator(c);

    SubareaRendererCreator *SUBAREARENDERER_CREATORS[] =
    {
        JitteredSubareaRendererCreator::GetInstancePtr(),
    };
    for(auto c : SUBAREARENDERER_CREATORS)
        SubareaRendererManager::GetInstance().AddCreator(c);
}

int main()
{
#if defined(_MSC_VER) && defined(_DEBUG)
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG)
                 | _CRTDBG_LEAK_CHECK_DF);
#endif

    std::locale::global(std::locale(""));

    InitializeObjectManagers();

    Config config;
    if(!config.LoadFromFile("./Build/scene.txt"))
    {
        cout << "Failed to load scene configuration..." << endl;
        return -1;
    }
    auto &conf = config.Root();

    ObjArena<> arena;
    auto &sceneMgr = SceneManager::GetInstance();

    try
    {
        auto filename = conf["output.filename"].AsValue();;
        auto width    = conf["output.width"] .AsValue().Parse<uint32_t>();
        auto height   = conf["output.height"].AsValue().Parse<uint32_t>();

        sceneMgr.Initialize(config.Root());

        //============= Render Target =============

        RenderTarget renderTarget(width, height);

        //============= Renderer & Integrator =============

        auto renderer        = RendererManager        ::GetInstance().GetSceneObject(conf["renderer"],        arena);
        auto subareaRenderer = SubareaRendererManager ::GetInstance().GetSceneObject(conf["subareaRenderer"], arena);
        auto integrator      = IntegratorManager      ::GetInstance().GetSceneObject(conf["integrator"],      arena);
        auto reporter        = ProgressReporterManager::GetInstance().GetSceneObject(conf["reporter"],        arena);

        //============= Post processor =============

        PostProcessor postProcessor;
        if(auto pps = conf.Find("postProcessors"))
        {
            auto &arrPP = pps->AsArray();
            for(size_t i = 0; i < arrPP.Size(); ++i)
                postProcessor.AddStage(PostProcessorStageManager::GetInstance().GetSceneObject(arrPP[i], arena));
        }

        //============= Rendering =============

        cout << "Start rendering..." << endl;

        Clock timer;
        renderer->Render(*subareaRenderer, sceneMgr.GetScene(), *integrator, &renderTarget, reporter);
        auto deltaTime = timer.Milliseconds() / 1000.0;

        cout << "Complete rendering...Total time: " << deltaTime << "s." << endl;

        //============= Output =============

        postProcessor.Process(renderTarget);
        TextureFile::WriteRGBToPNG(filename.ToStdWString(), ToSavedImage(renderTarget));
    }
    catch(const std::exception &err)
    {
        cout << err.what() << endl;
    }
    catch(...)
    {
        cout << "An unknown error occurred..." << endl;
    }
}
