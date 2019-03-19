#include <iostream>

#include <AGZUtils/Utils/Config.h>
#include <AGZUtils/Utils/Texture.h>
#include <Atrc/Lib/Core/TFilm.h>
#include <Atrc/Mgr/BuiltinCreatorRegister.h>
#include <Atrc/Mgr/Context.h>
#include <Atrc/Mgr/Parser.h>
#include <Atrc/Mgr/SceneBuilder.h>
#include <Atrc/SH2D/Scene2SH.h>
#include <Atrc/SH2D/Light2SH.h>

Atrc::Image Gamma(const Atrc::Image &img, Atrc::Real gamma)
{
    return img.Map([=](const Atrc::Spectrum &s)
    {
        return s.Map([=](Atrc::Real c)
        {
            return AGZ::Math::Pow(c, gamma);
        });
    });
}

void ProjectScene(const AGZ::Config &config, const std::string &configPath, const std::string &outputDir)
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

    if(SHOrder < 0 || SHOrder > 4)
    {
        std::cout << "Invalid SHOrder value: " << SHOrder << std::endl;
        return;
    }

    if(taskGridSize <= 0)
    {
        std::cout << "Invalid taskGridSize value: " << taskGridSize << std::endl;
        return;
    }

    int SHC = (SHOrder + 1) * (SHOrder + 1);
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

    auto saveToFile = [&](const std::string &filename, const Film &film)
    {
        auto img = film.GetImage().Map([](const Spectrum &pixel) { return pixel.ToFloats(); });
        std::ofstream fout(filename, std::ofstream::trunc | std::ofstream::binary);
        if(!fout)
            throw AGZ::Exception("Failed to open file: " + filename);
        AGZ::BinaryOStreamSerializer s(fout);
        if(!s.Serialize(img))
            throw AGZ::Exception("Failed to serialize into: " + filename);
    };

    auto outDir = std::filesystem::path(outputDir);

    for(int i = 0; i < SHC; ++i)
        saveToFile((outDir / ("coef" + std::to_string(i) + ".sh2d")).string(), coefs[i]);

    saveToFile((outDir / "binary.sh2d").string(), binary);
    AGZ::TextureFile::WriteRGBToHDR(
        (std::filesystem::path(outputDir) / "binary.hdr").string(),
        Gamma(binary.GetImage().Map([](const Spectrum &s) { return s.ToFloats(); }), Real(2.2)));

    saveToFile((outDir / "albedo.sh2d").string(), albedo);
    AGZ::TextureFile::WriteRGBToHDR(
        (std::filesystem::path(outputDir) / "albedo.hdr").string(),
        Gamma(albedo.GetImage().Map([](const Spectrum &s) { return s.ToFloats(); }), Real(2.2)));

    saveToFile((outDir / "normal.sh2d").string(), normal);
    AGZ::TextureFile::WriteRGBToHDR(
        (std::filesystem::path(outputDir) / "normal.hdr").string(),
        Gamma(normal.GetImage().Map([](const Spectrum &s) { return s.ToFloats(); }), Real(2.2)));
}

void ProjectLight(const AGZ::Config &config, const std::string &configPath, const std::string &outputFilename)
{
    using namespace Atrc;

    auto &root = config.Root();
    Mgr::Context ctx(root, configPath);
    Mgr::RegisterBuiltinCreators(ctx);

    auto light = ctx.Create<Light>(root["light"]);
    int N = root["N"].Parse<int>();

    int SHOrder = root["SHOrder"].Parse<int>();
    int SHC = (SHOrder + 1) * (SHOrder + 1);

    if(SHOrder < 0 || SHOrder > 4)
    {
        std::cout << "Invalid SHOrder value: " << SHOrder << std::endl;
        return;
    }

    std::vector<Spectrum> coefs(SHC);
    Light2SH(light, SHOrder, N, coefs.data());

    std::ofstream fout(outputFilename);
    if(!fout)
    {
        std::cout << "Failed to open output file " << outputFilename << std::endl;
        return;
    }

    fout << SHOrder << " ";
    for(int i = 0; i < SHC; ++i)
    {
        fout << coefs[i].r << " ";
        fout << coefs[i].g << " ";
        fout << coefs[i].b << " ";
    }

    if(!fout)
        std::cout << "Failed to save light coefs to " << outputFilename << std::endl;
}

void ReconstructImage(
    int SHOrder, const std::string &lightFilename,
    const std::string *sceneCoefFilenames,
    const std::string &albedoFilename,
    const std::string &outputFilename)
{
    using namespace Atrc;

    int SHC = (SHOrder + 1) * (SHOrder + 1);
    if(SHOrder < 0 || SHOrder > 4)
    {
        std::cout << "Invalid SHOrder value: " << SHOrder << std::endl;
        return;
    }

    std::ifstream fin(lightFilename);
    if(!fin)
    {
        std::cout << "Failed to open light coef file: " << lightFilename << std::endl;
        return;
    }

    int fileSHOrder;
    fin >> fileSHOrder;
    if(!fin || fileSHOrder < SHOrder)
    {
        std::cout << "Invalid light SHOrder in file: " << lightFilename << std::endl;
        return;
    }

    std::vector<Spectrum> lightCoefs(SHC);
    for(int i = 0; i < SHC; ++i)
    {
        fin >> lightCoefs[i].r >> lightCoefs[i].g >> lightCoefs[i].b;
        if(!fin)
        {
            std::cout << "Failed to load light coefs from file: " << lightFilename << std::endl;
            return;
        }
    }

    fin.close();

    auto loadImg = [&](const std::string &filename)->Image
    {
        std::ifstream fi(filename, std::ifstream::binary);
        if(!fi)
            throw AGZ::Exception("Failed to open file: " + filename);
        AGZ::BinaryIStreamDeserializer ds(fi);
        Image ret;
        if(!ds.Deserialize(ret))
            throw AGZ::Exception("Failed to deserialize image from: " + filename);
        return ret;
    };

    std::vector<Image> sceneCoefs(SHC);
    for(int i = 0; i < SHC; ++i)
    {
        sceneCoefs[i] = loadImg(sceneCoefFilenames[i]);
    }

    Image albedo;
    if(!albedoFilename.empty())
    {
        albedo = loadImg(albedoFilename);
        if(albedo.GetSize() != sceneCoefs[0].GetSize())
        {
            std::cout << "Invalid albedo image size" << std::endl;
            return;
        }
    }

    Image ret(albedo.GetWidth(), albedo.GetHeight());
    for(uint32_t y = 0; y < ret.GetHeight(); ++y)
    {
        for(uint32_t x = 0; x < ret.GetWidth(); ++x)
        {
            Spectrum pixel;
            for(int i = 0; i < SHC; ++i)
                pixel += sceneCoefs[i](x, y) * lightCoefs[i];
            if(albedo.IsAvailable())
                pixel *= albedo(x, y);
            ret(x, y) = pixel;
        }
    }

    AGZ::TextureFile::WriteRGBToPNG(outputFilename, ret.Map([](const Spectrum &spec)
    {
        return spec.Map([](Real c)
        {
            return static_cast<uint8_t>(AGZ::Math::Clamp<int>(static_cast<int>(c * 255), 0, 255));
        });
    }));
}

int main(int argc, char *argv[])
{
    try
    {
        static const char USAGE_MSG[] = R"___(
    SH2D ps/project_scene scene_desc_filename output_dir_name
    SH2D pl/project_light light_desc_filename output_filename
    SH2D rc/reconstruct   SH_order(0 to 4) light_coef_filename scene_coef_dir output_filename
)___";

        if(argc < 2)
        {
            std::cout << USAGE_MSG << std::endl;
            return 0;
        }

        if(argv[1] == std::string("ps") || argv[1] == std::string("project_scene"))
        {
            if(argc != 4)
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

            if(!std::filesystem::exists(outputDir) && !std::filesystem::create_directories(outputDir))
            {
                std::cout << "Failed to create output directory: " << outputDir << std::endl;
                return -1;
            }

            ProjectScene(config, absolute(std::filesystem::path(filename)).parent_path().string(), outputDir);
        }
        else if(argv[1] == std::string("pl") || argv[1] == std::string("project_light"))
        {
            if(argc != 4)
            {
                std::cout << USAGE_MSG << std::endl;
                return 0;
            }

            std::string filename = argv[2];
            std::string outputFilename = argv[3];
            auto outputDir = std::filesystem::path(outputFilename).parent_path();

            AGZ::Config config;
            if(!config.LoadFromFile(filename))
            {
                std::cout << "Failed to load light description file: " << filename << std::endl;
                return -1;
            }

            if(!exists(outputDir) && !create_directories(outputDir))
            {
                std::cout << "Failed to create output directory: " << outputDir << std::endl;
                return -1;
            }

            ProjectLight(config, absolute(std::filesystem::path(filename)).parent_path().string(), outputFilename);
        }
        else if(argv[1] == std::string("rc") || argv[1] == std::string("reconstruct"))
        {
            if(argc != 6)
            {
                std::cout << USAGE_MSG << std::endl;
                return 0;
            }

            int SHOrder = std::stoi(argv[2]);
            if(SHOrder < 0 || SHOrder > 4)
            {
                std::cout << USAGE_MSG << std::endl;
                return 0;
            }

            int SHC = (SHOrder + 1) * (SHOrder + 1);
            std::string lightFilename = argv[3];

            std::vector<std::string> sceneCoefFilenames(SHC);
            for(int i = 0; i < SHC; ++i)
                sceneCoefFilenames[i] = (std::filesystem::path(argv[4]) / ("coef" + std::to_string(i) + ".sh2d")).string();

            std::string outputFilename = argv[5];

            std::string albedoFilename = (std::filesystem::path(argv[4]) / "albedo.sh2d").string();

            ReconstructImage(SHOrder, lightFilename, sceneCoefFilenames.data(), albedoFilename, outputFilename);
        }
        else
            std::cout << USAGE_MSG << std::endl;
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
