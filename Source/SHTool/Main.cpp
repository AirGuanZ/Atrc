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
            return 0;
        }
        
        cout << USAGE_MSG << endl;
        return 0;
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

/* 文件结构：
    width:  uint32_t
    height: uint32_t
    {
        {
            {
                Spectrum: float * 3
            } * width
        } * height
    } * 9
 */
bool SaveProjectedEntity([[maybe_unused]] const Str8 &filename, [[maybe_unused]] const RenderTarget *renderTargets)
{
    
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

    ObjMgr::InitializeObjectManagers();
    ObjMgr::InitializePublicObjects(conf, arena);

    auto camera = ObjMgr::GetSceneObject<Atrc::Camera>(conf["camera"], arena);
    scene.camera = camera;

    // 取得实体列表

    auto &entArr = conf["entities"].AsArray();
    for(size_t i = 0; i < entArr.Size(); ++i)
    {
        auto ent = ObjMgr::EntityManager::GetInstance().GetSceneObject(entArr[i], arena);
        if(!ent)
            throw ObjMgr::SceneInitializationException("SceneManager: unknown entity type");
        scene.entities_.push_back(ent);
    }

    // 其他渲染配置项

    auto filename     = conf["output.filename"].AsValue();
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

    SHEntitySubareaRenderer subareaRenderer(spp, N);
    SHEntityRenderer renderer(workerCount);

    DefaultProgressReporter reporter;

    renderer.Render(subareaRenderer, scene, renderTargets.data(), &reporter);

    if(!SaveProjectedEntity(filename, renderTargets.data()))
        cout << "Failed to save rendered coefficients into " << filename.ToStdString() << endl;
}
