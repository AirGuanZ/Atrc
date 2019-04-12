#include <fstream>
#include <iostream>
#include <vector>

#include <Atrc/Core/Core/Common.h>
#include <Atrc/Core/Core/TFilm.h>
#include <Atrc/SH2D/ReconstructImage.h>
#include <Atrc/SH2D/RotateLight.h>

void ReconstructImage(
    int SHOrder, const std::string &lightFilename,
    const std::string *sceneCoefFilenames,
    const std::string &albedoFilename,
    const std::string &outputFilename,
    float rotateDeg)
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

    Mat3 rotateMat = Mat3::RotateY(Deg(rotateDeg));
    RotateLightCoefs(rotateMat, SHOrder, lightCoefs.data());

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
