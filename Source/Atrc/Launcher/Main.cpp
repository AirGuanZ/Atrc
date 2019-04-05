#include <filesystem>
#include <iostream>

#include <Atrc/Core/Core/PostProcessor.h>
#include <Atrc/Core/Core/Renderer.h>
#include <Atrc/Core/Core/Scene.h>
#include <Atrc/Core/Core/TFilm.h>
#include <Atrc/Mgr/BuiltinCreatorRegister.h>
#include <Atrc/Mgr/Context.h>
#include <Atrc/Mgr/Parser.h>
#include <Atrc/Mgr/SceneBuilder.h>

using namespace Atrc;

int Run(const AGZ::Config &config, std::string_view configPath)
{
    auto &root = config.Root();

    Mgr::Context context(root, configPath);
    Mgr::RegisterBuiltinCreators(context);

    auto renderer = context.Create<Renderer>  (root["renderer"]);
    auto sampler  = context.Create<Sampler>   (root["sampler"]);
    auto filter   = context.Create<FilmFilter>(root["film.filter"]);
    auto reporter = context.Create<Reporter>  (root["reporter"]);

    auto scene = Mgr::SceneBuilder::Build(root, context);

    auto outputFilename = context.GetPathInWorkspace(root["outputFilename"].AsValue());

    Vec2i filmSize;
    AGZ_HIERARCHY_TRY
    {
        filmSize = Mgr::Parser::ParseVec2i(root["film.size"]);
        if(filmSize.x <= 0 || filmSize.y <= 0)
            throw std::runtime_error("Invalid film size value");
    }
    AGZ_HIERARCHY_WRAP("In creating film")

    PostProcessorChain postProcessChain;
    AGZ_HIERARCHY_TRY
    {
        if(auto ppNode = root.Find("postProcessors"))
        {
            auto &arr = ppNode->AsArray();
            for(size_t i = 0; i < arr.Size(); ++i)
                postProcessChain.AddBack(context.Create<PostProcessor>(arr[i]));
        }
    }
    AGZ_HIERARCHY_WRAP("In creating post processors")

    Film film(filmSize, *filter);
    renderer->Render(&scene, sampler, &film, reporter);
    renderer->Join(reporter);

    auto image = film.GetImage();
    postProcessChain.Process(&image);

    AGZ::TextureFile::WriteRGBToPNG(outputFilename, image.Map(
    [](const Spectrum &c)
    {
        return c.Map([](Real s)
            { return uint8_t(Clamp<Real>(s * 256, 0, 255)); });
    }));

    return 0;
}

#if defined(_WIN32) && defined(_DEBUG)
#include <crtdbg.h>
#endif

const char *USAGE_MSG =
R"___(Usage:
    AtrcLauncher (load from ./Scene.txt)
    AtrcLauncher filename (load from filename)
    AtrcLauncher -m dummyConfigFilename content (load from string)
)___";

int main(int argc, char *argv[])
{
#if defined(_WIN32) && defined(_DEBUG)
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG)
        | _CRTDBG_LEAK_CHECK_DF);
#endif

    try
    {
        AGZ::Config config;
        std::string configPath;
        
        if(argc == 1)
        {
            configPath = ".";
            if(!config.LoadFromFile("./Scene.txt"))
                throw std::runtime_error("Failed to load configuration file from ./scene.txt");
        }
        else if(argc == 2)
        {
            if(std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help")
            {
                std::cout << USAGE_MSG;
                return 0;
            }

            configPath = std::filesystem::path(argv[1]).parent_path().string();
            if(!config.LoadFromFile(argv[1]))
                throw std::runtime_error("Failed to load configuration file from " + std::string(argv[1]));
        }
        else if(std::string(argv[1]) == "-m" && argc == 4)
        {
            configPath = argv[2];
            if(!config.LoadFromMemory(argv[3]))
                throw std::runtime_error("Failed to load configuration from memory");
        }
        else
        {
            std::cout << USAGE_MSG;
            return 0;
        }

        return Run(config, configPath);
    }
    catch(const std::exception &err)
    {
        std::vector<std::string> errMsgs;
        AGZ::ExtractHierarchyExceptions(err, std::back_inserter(errMsgs));
        for(auto &m : errMsgs)
            std::cout << m << std::endl;
    }
    catch(...)
    {
        std::cout << "An unknown error occurred..." << std::endl;
    }
}
