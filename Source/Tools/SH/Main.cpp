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
    shtool render_entity  [entity_project_result] [light_project_result] light_rotate_z_deg [output_filename])___";

void ProjectEntity(const Str8 &descFilename);
void ProjectLight(const Str8 &descFilename);
void RenderEntity(const Str8 &ent, const Str8 &light, Math::Radf lightRotateZ, const Str8 &output);

int main(int argc, char *argv[])
{
    try
    {
        if(argc < 2)
        {
            cout << USAGE_MSG << endl;
            return 0;
        }
        
        if(argv[1] == Str8("project_entity") && argc == 3)
        {
            ProjectEntity(argv[2]);
            return 0;
        }

        if(argv[1] == Str8("project_light") && argc == 3)
        {
            ProjectLight(argv[2]);
            return 0;
        }

        if(argv[1] == Str8("render_entity") && argc == 6)
        {
            RenderEntity(argv[2], argv[3], Math::Degf(Str8(argv[4]).Parse<float>()), argv[5]);
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

bool SaveProjectedEntity(const Str8 &filename, const RenderTarget *renderTargets)
{
    std::ofstream fout(filename.ToPlatformString());
    if(!fout)
        return false;
    BinaryOStreamSerializer serializer(fout);
    for(int i = 0; i < 9; ++i)
        serializer.Serialize(renderTargets[i]);
    return serializer.Ok();
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

bool SaveProjectedLight(const Str8 &filename, const Spectrum *output)
{
    std::ofstream fout(filename.ToPlatformString());
    if(!fout)
        return false;
    AGZ::BinaryOStreamSerializer serializer(fout);
    for(int i = 0; i < 9; ++i)
        serializer.Serialize(output[i]);
    return serializer.Ok();
}

void ProjectLight(const Str8 &descFilename)
{
    Config configFile;
    if(!configFile.LoadFromFile(descFilename))
    {
        cout << "Failed to load configuration from: " << descFilename.ToStdString() << endl;
        return;
    }

    auto &conf = configFile.Root();
    ObjArena<> arena;

    ObjMgr::InitializeObjectManagers();
    ObjMgr::InitializePublicObjects(conf, arena);

    auto filename = conf["outputFilename"].AsValue();
    auto N = conf["N"].AsValue().Parse<int>();
    
    if(N <= 0)
        throw ObjMgr::SceneInitializationException("Invalid N value");
    
    auto light = ObjMgr::GetSceneObject<Light>(conf["light"], arena);

    Spectrum output[9];
    SHLightProjector::Project(light, N, output);

    if(!SaveProjectedLight(filename, output))
        cout << "Failed to save rendered coefficients into " << filename.ToStdString() << endl;
}

bool LoadProjectedEntity(const Str8 &filename, RenderTarget *renderTargets)
{
    std::ifstream fin(filename.ToPlatformString());
    if(!fin)
        return false;

    AGZ::BinaryIStreamDeserializer deserializer(fin);
    for(int i = 0; i < 9; ++i)
    {
        if(!deserializer.Deserialize(renderTargets[i]))
            return false;
    }

    return true;
}

bool LoadProjectedLight(const Str8 &filename, Spectrum *output)
{
    std::ifstream fin(filename.ToPlatformString());
    if(!fin)
        return false;

    AGZ::BinaryIStreamDeserializer deserializer(fin);
    for(int i = 0; i < 9; ++i)
    {
        if(!deserializer.Deserialize(output[i]))
            return false;
    }

    return true;
}

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

void RenderEntity(const Str8 &ent, const Str8 &light, Math::Radf lightRotateZ, const Str8 &output)
{
    RenderTarget projectedEntity[9];
    Spectrum projectedLight[9];

    if(!LoadProjectedEntity(ent, projectedEntity))
    {
        cout << "Failed to load projected entity info from: " << ent.ToStdString() << endl;
        return;
    }

    if(!LoadProjectedLight(light, projectedLight))
    {
        cout << "Failed to load projected light info from: " << light.ToStdString() << endl;
        return;
    }

    float projectedLightR[9], projectedLightG[9], projectedLightB[9];
    for(int i = 0; i < 9; ++i)
    {
        projectedLightR[i] = projectedLight[i].r;
        projectedLightG[i] = projectedLight[i].g;
        projectedLightB[i] = projectedLight[i].b;
    }

    auto M = Math::Mat3f::RotateZ(lightRotateZ);
    
    Math::RotateSH_L0(M, &projectedLightR[0]);
    Math::RotateSH_L0(M, &projectedLightG[0]);
    Math::RotateSH_L0(M, &projectedLightB[0]);
    Math::RotateSH_L1(M, &projectedLightR[1]);
    Math::RotateSH_L1(M, &projectedLightG[1]);
    Math::RotateSH_L1(M, &projectedLightB[1]);
    Math::RotateSH_L2(M, &projectedLightR[4]);
    Math::RotateSH_L2(M, &projectedLightG[4]);
    Math::RotateSH_L2(M, &projectedLightB[4]);

    for(int i = 0; i < 9; ++i)
    {
        projectedLight[i].r = projectedLightR[i];
        projectedLight[i].g = projectedLightG[i];
        projectedLight[i].b = projectedLightB[i];
    }

    RenderTarget rt(projectedEntity[0].GetWidth(), projectedEntity[0].GetHeight());
    for(uint32_t y = 0; y < rt.GetHeight(); ++y)
    {
        for(uint32_t x = 0; x < rt.GetWidth(); ++x)
        {
            Spectrum pixel;
            for(int i = 0; i < 9; ++i)
                pixel += projectedEntity[i](x, y) * projectedLight[i];
            rt(x, y) = pixel;
        }
    }

    TextureFile::WriteRGBToPNG(output.ToStdWString(), ToSavedImage(rt));
}
