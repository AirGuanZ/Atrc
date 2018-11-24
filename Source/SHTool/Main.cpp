#include <iostream>

#include "ObjMgr/ObjectManager.h"
#include "SHProjector.h"

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

    // 输出配置

    [[maybe_unused]] auto filename     = conf["output.filename"].AsValue();
    auto outputWidth  = conf["output.width"].AsValue().Parse<uint32_t>();
    auto outputHeight = conf["output.height"].AsValue().Parse<uint32_t>();
    auto spp          = conf["spp"].AsValue().Parse<int>();
    auto N            = conf["N"].AsValue().Parse<int>();
    auto workerCount  = conf["workerCount"].AsValue().Parse<int>();

    if(spp <= 0 || N <= 0)
        throw ObjMgr::SceneInitializationException("Invalid spp/N value");

    std::vector<RenderTarget> renderTargets;
    renderTargets.reserve(9);
    for(int i = 0; i < 9; ++i)
        renderTargets.emplace_back(outputWidth, outputHeight);

    SHEntityProjector projector;
    SHSubareaRenderer subareaRenderer(spp, N);
    SHRenderer renderer(workerCount);

    DefaultProgressReporter reporter;

    renderer.Render(subareaRenderer, scene, projector, renderTargets.data(), &reporter);
}
