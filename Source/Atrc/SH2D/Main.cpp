#include <fstream>
#include <iostream>

#include <AGZUtils/Utils/Config.h>

#include <Atrc/Lib/Core/PostProcessor.h>

#include <Atrc/Mgr/BuiltinCreatorRegister.h>
#include <Atrc/Mgr/Context.h>
#include <Atrc/Mgr/Parser.h>
#include <Atrc/Mgr/SceneBuilder.h>

#include <Atrc/SH2D/LightProjector.h>
#include <Atrc/SH2D/ProjectResult.h>
#include <Atrc/SH2D/SceneProjector.h>

#if defined(_WIN32) && defined(_DEBUG)
#include <crtdbg.h>
#endif

using namespace Atrc;

void ProjectScene(const AGZ::Config &config, std::string_view configFilename)
{
    auto &root = config.Root();

    Mgr::Context context(root, configFilename);
    Mgr::RegisterBuiltinCreators(context);

    // 解析各种场景和投影参数

    auto sampler = context.Create<Sampler>(root["sampler"]);
    auto filter  = context.Create<FilmFilter>(root["film.filter"]);
    auto scene = Mgr::SceneBuilder::Build(root, context);

    auto outputFilename = context.GetPathInWorkspace(root["outputFilename"].AsValue());

    Vec2i filmSize;
    ATRC_MGR_TRY
    {
        filmSize = Mgr::Parser::ParseVec2i(root["film.size"]);
        if(filmSize.x <= 0 || filmSize.y <= 0)
            throw Mgr::MgrErr("Invalid film size value");
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating film")

    int SHOrder = root["SHOrder"].Parse<int>();
    if(SHOrder <= 0 || SHOrder > 5)
        throw Mgr::MgrErr("Invalid SHOrder value");
    int SHC = SHOrder * SHOrder;

    int workerCount = root["workerCount"].Parse<int>();
    int taskGridSize = root["taskGridSize"].Parse<int>();
    if(taskGridSize <= 0)
        throw Mgr::MgrErr("Invalid taskGridSize value");
    
    int minDepth = root["minDepth"].Parse<int>();
    int maxDepth = root["maxDepth"].Parse<int>();
    if(minDepth <= 0 || minDepth > maxDepth)
        throw Mgr::MgrErr("Invalid min/maxDepth value");

    Real contProb = root["contProb"].Parse<Real>();
    if(contProb <= 0 || contProb > 1)
        throw Mgr::MgrErr("Invalid contProb value");

    PostProcessorChain postProcessChain;
    ATRC_MGR_TRY
    {
        if(auto ppNode = root.Find("postProcessors"))
        {
            auto &arr = ppNode->AsArray();
            for(size_t i = 0; i < arr.Size(); ++i)
                postProcessChain.AddBack(context.Create<PostProcessor>(arr[i]));
        }
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating post processors")

    // 用于存放输出的films

    std::vector<Film> coefs;
    coefs.reserve(SHC);
    for(int i = 0; i < SHC; ++i)
        coefs.emplace_back(filmSize, *filter);
    TFilm<Real> binaryMask(filmSize, *filter);
    Film albedoMap(filmSize, *filter);
    Film normalMap(filmSize, *filter);

    SH2D::SceneProjector::ProjectResult films = {
        coefs.data(),
        &binaryMask,
        &albedoMap,
        &normalMap
    };

    // 投影

    SH2D::SceneProjector projector(
        workerCount, taskGridSize, SHOrder, minDepth, maxDepth, contProb);
    projector.Project(scene, sampler, &films);

    SH2D::SceneProjectResult result;
    result.SHC = SHC;
    result.coefs.reserve(SHC);
    for(int i = 0; i < SHC; ++i)
        result.coefs.push_back(coefs[i].GetImage());
    result.binaryMask = binaryMask.GetImage();
    result.albedoMap  = albedoMap.GetImage();
    result.normalMap  = normalMap.GetImage();

    // 后处理

    for(int i = 0; i < SHC; ++i)
        postProcessChain.Process(&result.coefs[i]);

    // 保存到文件

    std::ofstream fout(WIDEN(outputFilename), std::ofstream::trunc | std::ofstream::binary);
    if(!fout)
        throw Mgr::MgrErr("Failed to open output file: " + outputFilename);
    AGZ::BinaryOStreamSerializer serializer(fout);

    if(!serializer.Serialize(result))
        throw Mgr::MgrErr("Failed to serialize projected result");
}

void ProjectLight(const AGZ::Config &config, std::string_view configFilename)
{
    auto &root = config.Root();

    Mgr::Context context(root, configFilename);
    Mgr::RegisterBuiltinCreators(context);

    auto light = context.Create<Light>(root["light"]);

    int SHOrder = root["SHOrder"].Parse<int>();
    if(SHOrder <= 0 || SHOrder > 5)
        throw Mgr::MgrErr("Invalid SHOrder value");
    int SHC = SHOrder * SHOrder;

    int sampleCount = root["sampleCount"].Parse<int>();
    if(sampleCount <= 0)
        throw Mgr::MgrErr("Invalid sampleCount value");

    auto outputFilename = context.GetPathInWorkspace(root["outputFilename"].AsValue());
    
    std::vector<Spectrum> coefs(SHC);
    SH2D::LightProjector projector(SHOrder);
    projector.Project(light, sampleCount, coefs.data());

    SH2D::LightProjectResult result;
    result.SHC = SHC;
    result.coefs = std::move(coefs);

    // 保存到文件

    std::ofstream fout(WIDEN(outputFilename), std::ofstream::trunc | std::ofstream::binary);
    if(!fout)
        throw Mgr::MgrErr("Failed to open output file: " + outputFilename);
    AGZ::BinaryOStreamSerializer serializer(fout);

    if(!serializer.Serialize(result))
        throw Mgr::MgrErr("Failed to serialize projected result");
}

void ReconstructImage(const AGZ::Config &config, std::string_view configFilename)
{
    auto &root = config.Root();

    Mgr::Context context(root, configFilename);
    Mgr::RegisterBuiltinCreators(context);

    // 参数提取

    auto sceneFilename = context.GetPathInWorkspace(root["scene"].AsValue());
    auto lightFilename = context.GetPathInWorkspace(root["light"].AsValue());
    auto outputFilename = context.GetPathInWorkspace(root["outputFilename"].AsValue());
    auto rotateMat = Mgr::Parser::ParseRotateMat(root["rotation"]);

    // 数据加载

    SH2D::SceneProjectResult scene;
    {
        std::ifstream fin(WIDEN(sceneFilename), std::ifstream::binary);
        if(!fin)
            throw Mgr::MgrErr("Failed to open scene SH file: " + sceneFilename);
        AGZ::BinaryIStreamDeserializer deserializer(fin);
        if(!deserializer.Deserialize(scene))
            throw Mgr::MgrErr("Failed to deserialize scene SH coefs");
    }

    SH2D::LightProjectResult light;
    {
        std::ifstream fin(WIDEN(lightFilename), std::ifstream::binary);
        if(!fin)
            throw Mgr::MgrErr("Failed to open light SH file: " + lightFilename);
        AGZ::BinaryIStreamDeserializer deserializer(fin);
        if(!deserializer.Deserialize(light))
            throw Mgr::MgrErr("Failed to deserialize light SH coefs");
    }

    // 旋转光源

    light.Rotate(rotateMat);

    // 重建图像

    auto SHC = Min(scene.SHC, light.SHC);
    auto image = SH2D::ReconstructFromSH(SHC, scene.coefs.data(), light.coefs.data());

    // 保存到文件

    AGZ::TextureFile::WriteRGBToPNG(outputFilename, image.Map(
    [](const Spectrum &c)
    {
        return c.Map([](Real s)
            { return uint8_t(Clamp<Real>(s * 256, 0, 255)); });
    }));
}

const char *USAGE_MSG =
R"___(Usage:
    SH2D ps|project_scene filename
    SH2D ps|project_scene -m dummyConfigFilename content
    SH2D pl|project_light filename
    SH2D pl|project_light -m dummyConfigFilename content
    SH2D rc|reconstruct filename
    SH2D rc|reconstruct -m dummyConfigFilename content
)___";

int main(int argc, char *argv[])
{
#if defined(_WIN32) && defined(_DEBUG)
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG)
        | _CRTDBG_LEAK_CHECK_DF);
#endif

    try
    {
        if(argc < 2)
        {
            std::cout << USAGE_MSG;
            return 0;
        }
        
        std::string funcName(argv[1]);

        if(funcName == "ps" || funcName == "project_scene")
        {
            std::string configFilename;
            AGZ::Config config;

            if(argc == 3)
            {
                configFilename = argv[2];
                if(!config.LoadFromFile(configFilename))
                    throw Mgr::MgrErr("Failed to load configuration file from " + configFilename);
            }
            else if(argc == 5 && std::string(argv[2]) == "-m")
            {
                configFilename = argv[3];
                if(!config.LoadFromMemory(argv[4]))
                    throw Mgr::MgrErr("Failed to load configutation from memory");
            }
            else
            {
                std::cout << USAGE_MSG;
                return 0;
            }

            ProjectScene(config, configFilename);
            return 0;
        }

        if(funcName == "pl" || funcName == "project_light")
        {
            std::string configFilename;
            AGZ::Config config;

            if(argc == 3)
            {
                configFilename = argv[2];
                if(!config.LoadFromFile(configFilename))
                    throw Mgr::MgrErr("Failed to load configuration file from " + configFilename);
            }
            else if(argc == 5 && std::string(argv[2]) == "-m")
            {
                configFilename = argv[3];
                if(!config.LoadFromMemory(argv[4]))
                    throw Mgr::MgrErr("Failed to load configutation from memory");
            }
            else
            {
                std::cout << USAGE_MSG;
                return 0;
            }

            ProjectLight(config, configFilename);
            return 0;
        }

        if(funcName == "rc" || funcName == "reconstruct")
        {
            std::string configFilename;
            AGZ::Config config;

            if(argc == 3)
            {
                configFilename = argv[2];
                if(!config.LoadFromFile(configFilename))
                    throw Mgr::MgrErr("Failed to load configuration file from " + configFilename);
            }
            else if(argc == 5 && std::string(argv[2]) == "-m")
            {
                configFilename = argv[3];
                if(!config.LoadFromMemory(argv[4]))
                    throw Mgr::MgrErr("Failed to load configutation from memory");
            }
            else
            {
                std::cout << USAGE_MSG;
                return 0;
            }

            ReconstructImage(config, configFilename);
            return 0;
        }

        std::cout << USAGE_MSG;
        return 0;
    }
    catch(const Mgr::MgrErr &err)
    {
        for(auto pErr = &err; pErr; pErr = pErr->TryGetInterior())
        {
            std::cout << pErr->GetMsg() << std::endl;
            if(pErr->TryGetLeaf())
            {
                std::cout << pErr->TryGetLeaf()->what() << std::endl;
                break;
            }
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
