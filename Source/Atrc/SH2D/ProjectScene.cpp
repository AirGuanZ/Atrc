#include <AGZUtils/Utils/Config.h>
#include <AGZUtils/Utils/Texture.h>
#include <Atrc/Core/Core/TFilm.h>
#include <Atrc/Mgr/BuiltinCreatorRegister.h>
#include <Atrc/Mgr/Context.h>
#include <Atrc/Mgr/Parser.h>
#include <Atrc/Mgr/SceneBuilder.h>
#include <Atrc/SH2D/ProjectScene.h>
#include <Atrc/SH2D/Scene2SH.h>
#include <Lib/cnpy/cnpy.h>

namespace
{
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
}

void ProjectScene(const AGZ::Config &config, const std::string &configPath, const ProjectSceneOutputArgs &outputArgs)
{
    using namespace Atrc;

    auto &root = config.Root();
    Mgr::Context ctx(root, configPath);
    Mgr::RegisterBuiltinCreators(ctx);

    auto sampler = ctx.Create<Sampler>(root["sampler"]);
    auto filter = ctx.Create<FilmFilter>(root["film.filter"]);
    auto scene = Mgr::SceneBuilder::Build(root, ctx);

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

    auto &dir = outputArgs.dir;

    std::vector<AGZ::Texture2D<AGZ::Math::Vec3f>> imgs(coefs.size());
    for(size_t i = 0; i < coefs.size(); ++i)
        imgs[i] = coefs[i].GetImage().Map([](const Spectrum &pixel) { return pixel.ToFloats(); });

    if(outputArgs.outputSH2D)
    {
        auto saveToFile = [&](const std::string &filename, const auto &img)
        {
            std::ofstream fout(filename, std::ofstream::trunc | std::ofstream::binary);
            if(!fout)
                throw AGZ::Exception("Failed to open file: " + filename);
            AGZ::BinaryOStreamSerializer s(fout);
            if(!s.Serialize(img))
                throw AGZ::Exception("Failed to serialize into: " + filename);
        };

        saveToFile((dir / "binary.sh2d").string(), binary.GetImage());
        saveToFile((dir / "albedo.sh2d").string(), albedo.GetImage());
        saveToFile((dir / "normal.sh2d").string(), normal.GetImage());
        for(int i = 0; i < SHC; ++i)
            saveToFile((dir / ("coef" + std::to_string(i) + ".sh2d")).string(), imgs[i]);
    }

    if(outputArgs.outputHDR)
    {
        AGZ::TextureFile::WriteRGBToHDR(
            (dir / "binary.hdr").string(),
            Gamma(binary.GetImage().Map([](const Spectrum &s) { return s.ToFloats(); }), Real(2.2)));

        AGZ::TextureFile::WriteRGBToHDR(
            (dir / "albedo.hdr").string(),
            Gamma(albedo.GetImage().Map([](const Spectrum &s) { return s.ToFloats(); }), Real(2.2)));

        AGZ::TextureFile::WriteRGBToHDR(
            (dir / "normal.hdr").string(),
            Gamma(normal.GetImage().Map([](const Spectrum &s) { return ((s + Spectrum(1)) * Atrc::Real(0.5)).ToFloats(); }), Real(2.2)));

        AGZ::TextureFile::WriteRGBToPNG(
            (dir / "binary.png").string(), binary.GetImage().Map([](const Spectrum &s)
            {
                return s.Map([](Atrc::Real c)
                {
                    return uint8_t(AGZ::Math::Clamp<Atrc::Real>(c * Atrc::Real(255), 0, 255));
                });
            })
        );

        AGZ::TextureFile::WriteRGBToPNG(
            (dir / "albedo.png").string(), albedo.GetImage().Map([](const Spectrum &s)
            {
                return s.Map([](Atrc::Real c)
                {
                    return uint8_t(AGZ::Math::Clamp<Atrc::Real>(c * Atrc::Real(255), 0, 255));
                });
            })
        );

        AGZ::TextureFile::WriteRGBToPNG(
            (dir / "normal.png").string(), normal.GetImage().Map([](const Spectrum &s)
            {
                return s.Map([](Atrc::Real c)
                {
                    return uint8_t(AGZ::Math::Clamp<Atrc::Real>((c + 1) / 2 * Atrc::Real(255), 0, 255));
                });
            })
        );

        for(int i = 0; i < SHC; ++i)
        {
            AGZ::TextureFile::WriteRGBToHDR(
                (dir / ("coef" + std::to_string(i) + ".hdr")).string(), Gamma(imgs[i], Real(2.2)));
        }
    }

    if(outputArgs.outputNPZ)
    {
        auto w = imgs[0].GetWidth(), h = imgs[0].GetHeight();

        {
            std::vector<AGZ::Math::Vec3d> albedoData(w * h);
            auto normalData = albedoData, binaryData = albedoData;

            auto albedoImg = albedo.GetImage(), normalImg = normal.GetImage(), binaryImg = binary.GetImage();;

            for(uint32_t y = 0; y < h; ++y)
            {
                auto yBase = y * w;
                for(uint32_t x = 0; x < w; ++x)
                {
                    auto idx = yBase + x;
                    albedoData[idx] = albedoImg(x, y).ToDoubles();
                    normalData[idx] = normalImg(x, y).ToDoubles();
                    binaryData[idx] = binaryImg(x, y).ToDoubles();
                }
            }

            cnpy::npz_save((dir / "albedo.npz").string(), "albedo", &albedoData[0][0], { h, w, 3 });
            cnpy::npz_save((dir / "normal.npz").string(), "normal", &normalData[0][0], { h, w, 3 });
            cnpy::npz_save((dir / "binary.npz").string(), "binary", &binaryData[0][0], { h, w, 3 });
        }

        {
            std::vector<float> data;
            data.reserve(w * h * SHC);
            for(uint32_t y = 0; y < h; ++y)
            {
                for(uint32_t x = 0; x < w; ++x)
                {
                    for(int i = 0; i < SHC; ++i)
                        data.push_back(imgs[i](x, y).ToFloats().r);
                }
            }
            cnpy::npz_save((dir / "coefs.npz").string(), "coefs", &data[0], { h, w, static_cast<size_t>(SHC) });
        }
    }
}
