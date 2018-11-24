#include <iostream>

#include "ObjMgr/Common.h"
#include "ObjMgr/ObjectManager/CameraManager.h"
#include "ObjMgr/ObjectManager.h"

using namespace AGZ;
using namespace Atrc;
using namespace std;

const char *USAGE_MSG =
R"___(Usage:
    shtool project_entity [entity_desc_filename]
    shtool project_light  [light_desc_filename]
    shtool render_entity  [entity_project_result] [light_project_result] (-o [output_filename])
    shtool render_light   [light_project_result] (-o [output_filename]))___";

void ProjectEntity(const Str8 &descFilename);

int main(int argc, char *argv[])
{
    try
    {
        if(argc < 2)
        {
            cout << USAGE_MSG << endl;
            return 0;
        }

        if(argv[1] == Str8("project_entity"))
        {
            if(argc == 3)
            {
                Str8 descFilename(argv[2]);
                ProjectEntity(descFilename);
            }
        }
    }
    catch(std::exception &err)
    {
        cout << err.what() << endl;
    }
    catch(...)
    {
        cout << "An unknown error occurred..." << endl;
    }
}

void ProjectEntity(const Str8 &descFilename)
{
    Config configFile;
    if(!configFile.LoadFromFile(descFilename))
    {
        cout << "Failed to load configuration from: " << descFilename.ToStdString() << endl;
        return;
    }

    auto &conf = configFile.Root();

    ObjArena<> arena;
    Scene scene;

    InitializeObjectManagers();

    // 预定义元素

    ObjMgr::InitializePublicDefinition<Geometry>("pub_geometry", conf, arena);
    ObjMgr::InitializePublicDefinition<Material>("pub_material", conf, arena);
    ObjMgr::InitializePublicDefinition<Light>   ("pub_light", conf, arena);
    ObjMgr::InitializePublicDefinition<Medium>  ("pub_medium", conf, arena);
    ObjMgr::InitializePublicDefinition<Entity>  ("pub_entity", conf, arena);

    ObjMgr::InitializePublicDefinition<Camera>          ("pub_geometry", conf, arena);
    ObjMgr::InitializePublicDefinition<Renderer>        ("pub_renderer", conf, arena);
    ObjMgr::InitializePublicDefinition<ProgressReporter>("pub_reporter", conf, arena);

    auto camera = ObjMgr::CameraManager::GetInstance().GetSceneObject(conf["camera"], arena);
    scene.camera = camera;

    // 创建实体

    auto &entArr = conf["entities"].AsArray();
    for(size_t i = 0; i < entArr.Size(); ++i)
    {
        auto ent = ObjMgr::EntityManager::GetInstance().GetSceneObject(entArr[i], arena);
        if(!ent)
            throw ObjMgr::SceneInitializationException("SceneManager: unknown entity type");
        scene.entities_.push_back(ent);
    }

    // 其他配置元素

    auto renderer        = ObjMgr::GetSceneObject<Renderer>        (conf["renderer"],        arena);
    auto subareaRenderer = ObjMgr::GetSceneObject<SubareaRenderer> (conf["subareaRenderer"], arena);

    // 输出配置

    [[maybe_unused]] auto filename     = conf["output.filename"].AsValue();
    auto outputWidth  = conf["output.width"].AsValue().Parse<uint32_t>();
    auto outputHeight = conf["output.height"].AsValue().Parse<uint32_t>();
    auto sampleCount  = conf["sampleCount"].AsValue().Parse<int>();

    if(sampleCount <= 0)
        throw ObjMgr::SceneInitializationException("Invalid sampleCount value");

    std::vector<RenderTarget> renderTargets;

    for(int L = 0; L < 3; ++L)
    {
        for(int M = -L; M <= L; ++M)
        {
            renderTargets.emplace_back(outputWidth, outputHeight);
            auto &renderTarget = renderTargets.back();

            cout << "Start evaluating coefficient for (L, M) = (" << L << ", " << M << ")..." << endl;

            Clock timer;
            SHEntityProjector integrator(L, M, sampleCount);
            renderer->Render(*subareaRenderer, scene, integrator, &renderTarget, nullptr);
            auto deltaTime = timer.Milliseconds() / 1000.0;

            cout << "Complete evaluating...Total time: " << deltaTime << "s." << endl;
        }
    }
}
