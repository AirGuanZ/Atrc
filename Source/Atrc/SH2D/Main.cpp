#include <iostream>

#include <AGZUtils/Utils/Config.h>
#include <AGZUtils/Utils/Texture.h>
#include <Atrc/Lib/Core/TFilm.h>
#include <Atrc/Mgr/BuiltinCreatorRegister.h>
#include <Atrc/Mgr/Context.h>
#include <Atrc/Mgr/Parser.h>
#include <Atrc/Mgr/SceneBuilder.h>
#include <Atrc/SH2D/Scene2SH.h>

void ProjectScene(const AGZ::Config &config, std::string_view configPath, std::string_view outputDir)
{
    using namespace Atrc;

    auto &root = config.Root();
    Mgr::Context ctx(root, configPath);
    Mgr::RegisterBuiltinCreators(ctx);

    auto sampler = ctx.Create<Sampler>(root["sampler"]);
    auto filter  = ctx.Create<FilmFilter>(root["film.filter"]);
    auto scene   = Mgr::SceneBuilder::Build(root, ctx);

    Vec2i filmSize = Mgr::Parser::ParseVec2i(root["film.size"]);

    int SHOrder = root["SHOrder"].Parse<int>();
    int workerCount = root["workerCount"].Parse<int>();
    int taskGridSize = root["taskGridSize"].Parse<int>();

    int SHC = SHOrder * SHOrder;
    std::vector<Film> coefs;
    coefs.reserve(SHC);
    for(int i = 0; i < SHC; ++i)
        coefs.emplace_back(filmSize, *filter);
    
    Film binary(filmSize, *filter);
    Film albedo(filmSize, *filter);
    Film normal(filmSize, *filter);

    Scene2SHResult result =
    {
        coefs.data(),
        &binary,
        &albedo,
        &normal
    };

    Scene2SH(workerCount, taskGridSize, SHOrder, scene, sampler, &result);

    auto saveToHDR = [&](const std::string &filename, const Film &film)
    {
        auto img = film.GetImage().Map([](const Spectrum &pixel) { return pixel.ToFloats(); });
        AGZ::TextureFile::WriteRGBToHDR((std::filesystem::path(outputDir) / filename).string(), img);
    };

    for(int i = 0; i < SHC; ++i)
        saveToHDR("coef" + std::to_string(i) + ".hdr", coefs[i]);
    saveToHDR("binary.hdr", binary);
    saveToHDR("albedo.hdr", albedo);
    saveToHDR("normal.hdr", normal);
}

int main(int argc, char *argv[])
{
    try
    {
        static const char USAGE_MSG[] = R"___(
    SH2D ps/project_scene scene_desc_filename output_dir_name
)___";

        if(argc != 4)
        {
            std::cout << USAGE_MSG << std::endl;
            return 0;
        }

        if(argv[1] != std::string("ps") || argv[1] != std::string("project_scene"))
        {
            std::cout << USAGE_MSG << std::endl;
            return 0;
        }

        std::string filename = argv[2];
        std::string outputDir = argv[3];

        AGZ::Config config;
        if(!config.LoadFromFile(filename))
        {
            std::cout << "Failed to load scene description file: " << filename << std::endl;
            return -1;
        }

        if(!std::filesystem::exists(outputDir))
        {
            std::cout << "output directory doesn't exist: " << outputDir << std::endl;
            return -1;
        }

        ProjectScene(config, absolute(std::filesystem::path(outputDir)).parent_path().string(), outputDir);
    }
    catch(const Atrc::Mgr::MgrErr &err)
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
