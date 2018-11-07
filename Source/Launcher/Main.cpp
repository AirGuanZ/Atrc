#include <iostream>

#include <Atrc/Atrc.h>
#include <Utils.h>

#if defined(_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>
#endif

using namespace AGZ;
using namespace Atrc;
using namespace std;

// See https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
float ACESFilm(float x)
{
    /*constexpr float a = 2.51f;
    constexpr float b = 0.03f;
    constexpr float c = 2.43f;
    constexpr float d = 0.59f;
    constexpr float e = 0.14f;
    return Clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0f, 1.0f);*/
    return x;
}

Texture2D<Color3b> ToSavedImage(const RenderTarget &origin, float gamma)
{
    return origin.Map([=](const Color3f &color)
    {
        return color.Map([=](float x)
        {
            return static_cast<uint8_t>(Clamp(Pow(ACESFilm(x), gamma), 0.0f, 1.0f) * 255);
        });
    });
}

#include "EntityManager/EntityCreator.h"
#include "GeometryManager/GeometryManager.h"
#include "LightManager/LightManager.h"
#include "IntegratorManager/IntegratorManager.h"
#include "MaterialManager/MaterialManager.h"
#include "MediumManager/MediumManager.h"
#include "SceneManager/SceneManager.h"

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
        DirectionalLightCreator::GetInstancePtr(),
        SkyLightCreator::GetInstancePtr(),
    };
    for(auto c : LIGHT_CREATORS)
        LightManager::GetInstance().AddCreator(c);

    IntegratorCreator *INTEGRATOR_CREATORS[] =
    {
        PathTracerCreator::GetInstancePtr(),
        PureColorIntegratorCreator::GetInstancePtr(),
        VolumetricPathTracerCreator::GetInstancePtr(),
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
}

int main()
{
#if defined(_MSC_VER) && defined(_DEBUG)
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG)
				 | _CRTDBG_LEAK_CHECK_DF);
#endif

    InitializeObjectManagers();

	Config config;
	if(!config.LoadFromFile("./Build/scene.txt"))
	{
		cout << "Failed to load scene configuration..." << endl;
		return -1;
	}
    auto &conf = config.Root();

    ObjArena<> arena;

    uint32_t width, height, spp;
    auto &sceneMgr = SceneManager::GetInstance();

    try
    {
        width  = conf["output.width"].AsValue().Parse<uint32_t>();
        height = conf["output.height"].AsValue().Parse<uint32_t>();
        spp    = conf["spp"].AsValue().Parse<uint32_t>();

        sceneMgr.Initialize(config.Root());
    }
    catch(const std::exception &err)
    {
        cout << "Failed to initialize scene. " << err.what() << endl;
        return -1;
    }

	//============= Render Target =============

	RenderTarget renderTarget(width, height);

	//============= Renderer & Integrator =============

	JitteredSubareaRenderer subareaRenderer(spp);

	ParallelRenderer renderer(6);
	//SerialRenderer renderer;
	renderer.EnableProgressPrinting(true);

    auto integrator = IntegratorManager::GetInstance().Create(conf["integrator"].AsGroup(), arena);

	//============= Rendering =============

	cout << "Start rendering..." << endl;

	Timer timer;
	renderer.Render(subareaRenderer, sceneMgr.GetScene(), *integrator, renderTarget);
	auto deltaTime = timer.Milliseconds() / 1000.0;

	cout << "Complete rendering...Total time: " << deltaTime << "s." << endl;

	//============= Output =============

	TextureFile::WriteRGBToPNG("./Build/Output.png", ToSavedImage(renderTarget, 1 / 2.2f));
}
