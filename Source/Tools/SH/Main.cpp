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
    shtool render_entity  [entity_project_result] [light_project_result] bands light_rotate_z_deg [output_filename])___";

void Run(int argc, char *argv[]);
void ProjectEntity(const Str8 &descFilename);
void ProjectLight(const Str8 &descFilename);
void RenderEntity(const Str8 &ent, const Str8 &light, int bands, Math::Radf lightRotateZ, const Str8 &output);

int main(int argc, char *argv[])
{
#ifndef _DEBUG
    try
    {
        Run(argc, argv);
    }
    catch(std::exception &err)
    {
        cout << err.what() << endl;
    }
    catch(...)
    {
        cout << "An unknown error occurred..." << endl;
    }
#else
    Run(argc, argv);
#endif

    return 0;
}

void Run(int argc, char *argv[])
{
    if(argc < 2)
    {
        cout << USAGE_MSG << endl;
        return;
    }

    if(argv[1] == Str8("project_entity") && argc == 3)
    {
        ProjectEntity(argv[2]);
        return;
    }

    if(argv[1] == Str8("project_light") && argc == 3)
    {
        ProjectLight(argv[2]);
        return;
    }

    if(argv[1] == Str8("render_entity") && argc == 7)
    {
        RenderEntity(argv[2], argv[3], Str8(argv[4]).Parse<int>(), Math::Degf(Str8(argv[5]).Parse<float>()), argv[6]);
        return;
    }

    cout << USAGE_MSG << endl;
}

bool SaveProjectedEntity(const Str8 &filename, const RenderTarget *renderTargets, int SHC)
{
    std::ofstream fout(filename.ToPlatformString(), std::ios::binary | std::ios::trunc);
    if(!fout)
        return false;
    BinaryOStreamSerializer serializer(fout);
    for(int i = 0; i < SHC; ++i)
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

    auto camera = ObjMgr::GetSceneObject<Camera>(conf["camera"], arena);
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
    auto bands        = conf["Bands"].AsValue().Parse<int>();
    auto workerCount  = conf["workerCount"].AsValue().Parse<int>();

    auto SHC = bands * bands;

    if(spp <= 0 || N <= 0 || bands <= 0)
        throw ObjMgr::SceneInitializationException("Invalid spp/N/Bands value");

    std::vector<RenderTarget> renderTargets;
    renderTargets.reserve(SHC);
    for(int i = 0; i < SHC; ++i)
        renderTargets.emplace_back(outputWidth, outputHeight);

    SHEntitySubareaRenderer subareaRenderer(spp, SHC, N);
    SHEntityRenderer renderer(workerCount);

    DefaultProgressReporter reporter;

    renderer.Render(subareaRenderer, scene, renderTargets.data(), &reporter);

    if(!SaveProjectedEntity(filename, renderTargets.data(), SHC))
        cout << "Failed to save rendered coefficients into " << filename.ToStdString() << endl;
}

bool SaveProjectedLight(const Str8 &filename, const Spectrum *output, int SHC)
{
    std::ofstream fout(filename.ToPlatformString(), std::ios::binary | std::ios::trunc);
    if(!fout)
        return false;
    BinaryOStreamSerializer serializer(fout);
    for(int i = 0; i < SHC; ++i)
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
    auto N     = conf["N"].AsValue().Parse<int>();
    auto bands = conf["Bands"].AsValue().Parse<int>();

    auto SHC = bands * bands;

    if(N <= 0 || bands <= 0)
        throw ObjMgr::SceneInitializationException("Invalid N/Bands value");
    
    auto light = ObjMgr::GetSceneObject<Light>(conf["light"], arena);

    vector<Spectrum> output(SHC);
    SHLightProjector::Project(light, SHC, N, output.data());

    if(!SaveProjectedLight(filename, output.data(), SHC))
        cout << "Failed to save rendered coefficients into " << filename.ToStdString() << endl;
}

bool LoadProjectedEntity(const Str8 &filename, int SHC, RenderTarget *renderTargets)
{
    std::ifstream fin(filename.ToPlatformString(), std::ios::binary);
    if(!fin)
        return false;
    BinaryIStreamDeserializer deserializer(fin);
    for(int i = 0; i < SHC; ++i)
    {
        if(!deserializer.Deserialize(renderTargets[i]))
            return false;
    }
    return true;
}

bool LoadProjectedLight(const Str8 &filename, int SHC, Spectrum *output)
{
    std::ifstream fin(filename.ToPlatformString(), std::ios::binary);
    if(!fin)
        return false;
    BinaryIStreamDeserializer deserializer(fin);
    for(int i = 0; i < SHC; ++i)
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

void RenderEntity(const Str8 &ent, const Str8 &light, int bands, Math::Radf lightRotateZ, const Str8 &output)
{
    if(bands <= 0 || bands > 5)
    {
        cout << "Invalid SHC value" << endl;
        return;
    }

    int SHC = bands * bands;

    vector<RenderTarget> projectedEntity(SHC);
    vector<Spectrum> projectedLight(SHC);

    if(!LoadProjectedEntity(ent, SHC, projectedEntity.data()))
    {
        cout << "Failed to load projected entity info from: " << ent.ToStdString() << endl;
        return;
    }

    if(!LoadProjectedLight(light, SHC, projectedLight.data()))
    {
        cout << "Failed to load projected light info from: " << light.ToStdString() << endl;
        return;
    }

    vector<float> projectedLightR(SHC), projectedLightG(SHC), projectedLightB(SHC);

    for(int i = 0; i < SHC; ++i)
    {
        projectedLightR[i] = projectedLight[i].r;
        projectedLightG[i] = projectedLight[i].g;
        projectedLightB[i] = projectedLight[i].b;
    }

    auto M = Math::Mat3f::RotateZ(lightRotateZ);

    Math::RotateSH_L0(M, &projectedLightR[0]);
    Math::RotateSH_L0(M, &projectedLightG[0]);
    Math::RotateSH_L0(M, &projectedLightB[0]);
    if(SHC >= 4)
    {
        Math::RotateSH_L1(M, &projectedLightR[1]);
        Math::RotateSH_L1(M, &projectedLightG[1]);
        Math::RotateSH_L1(M, &projectedLightB[1]);
        if(SHC >= 9)
        {
            Math::RotateSH_L2(M, &projectedLightR[4]);
            Math::RotateSH_L2(M, &projectedLightG[4]);
            Math::RotateSH_L2(M, &projectedLightB[4]);
            if(SHC >= 16)
            {
                Math::RotateSH_L3(M, &projectedLightR[9]);
                Math::RotateSH_L3(M, &projectedLightG[9]);
                Math::RotateSH_L3(M, &projectedLightB[9]);
                if(SHC >= 25)
                {
                    Math::RotateSH_L4(M, &projectedLightR[16]);
                    Math::RotateSH_L4(M, &projectedLightG[16]);
                    Math::RotateSH_L4(M, &projectedLightB[16]);
                }
            }
        }
    }

    for(int i = 0; i < SHC; ++i)
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
            for(int i = 0; i < SHC; ++i)
                pixel += projectedEntity[i](x, y) * projectedLight[i];
            rt(x, y) = pixel;
        }
    }

    TextureFile::WriteRGBToPNG(output, ToSavedImage(rt));
}
