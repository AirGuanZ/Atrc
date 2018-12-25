#include <iostream>

#include <Utils/Config.h>

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

void ProjectScene(const AGZ::Config &config, const Str8 &configFilename)
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
    if(SHOrder <= 0 || SHOrder > 4)
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

    // 保存到文件

    std::ofstream fout(outputFilename.ToPlatformString(),
                       std::ofstream::trunc | std::ofstream::binary);
    if(!fout)
        throw Mgr::MgrErr("Failed to open output file: " + outputFilename);
    AGZ::BinaryOStreamSerializer serializer(fout);

    if(!serializer.Serialize(result))
        throw Mgr::MgrErr("Failed to serialize projected result");
}

void ProjectLight(const AGZ::Config &config, const Str8 &configFilename)
{
    auto &root = config.Root();

    Mgr::Context context(root, configFilename);
    Mgr::RegisterBuiltinCreators(context);

    auto light = context.Create<Light>(root["light"]);

    int SHOrder = root["SHOrder"].Parse<int>();
    if(SHOrder <= 0 || SHOrder > 4)
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

    std::ofstream fout(outputFilename.ToPlatformString(),
                       std::ofstream::trunc | std::ofstream::binary);
    if(!fout)
        throw Mgr::MgrErr("Failed to open output file: " + outputFilename);
    AGZ::BinaryOStreamSerializer serializer(fout);

    if(!serializer.Serialize(result))
        throw Mgr::MgrErr("Failed to serialize projected result");
}

const char *USAGE_MSG =
R"___(Usage:
    SH2D ps|project_scene filename
    SH2D ps|project_scene -m dummyConfigFilename content
    SH2D pl|project_light filename
    SH2D pl|project_light -m dummyConfigFilename content
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
        
        Str8 funcName(argv[1]);

        if(funcName == "ps" || funcName == "project_scene")
        {
            Str8 configFilename;
            AGZ::Config config;

            if(argc == 3)
            {
                configFilename = argv[2];
                if(!config.LoadFromFile(configFilename))
                    throw Mgr::MgrErr("Failed to load configuration file from " + configFilename);
            }
            else if(argc == 5 && Str8(argv[2]) == "-m")
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
            Str8 configFilename;
            AGZ::Config config;

            if(argc == 3)
            {
                configFilename = argv[2];
                if(!config.LoadFromFile(configFilename))
                    throw Mgr::MgrErr("Failed to load configuration file from " + configFilename);
            }
            else if(argc == 5 && Str8(argv[2]) == "-m")
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

        std::cout << USAGE_MSG;
        return 0;
    }
    catch(const Mgr::MgrErr &err)
    {
        for(auto pErr = &err; pErr; pErr = pErr->TryGetInterior())
        {
            std::cout << pErr->GetMsg().ToStdString() << std::endl;
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
