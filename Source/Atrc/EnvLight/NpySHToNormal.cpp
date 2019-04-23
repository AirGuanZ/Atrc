#include <iostream>
#include <string>

#include <AGZUtils/Utils/Math.h>
#include <AGZUtils/Utils/Texture.h>
#include <Lib/cnpy/cnpy.h>

using namespace AGZ::Math;

namespace
{
    std::pair<int, std::vector<Vec3f>> LoadSHFromNPY(const std::string &npyFilename)
    {
        auto arr = cnpy::npy_load(npyFilename);
        if(arr.shape.size() != 2 || arr.shape[1] != 3 || arr.word_size != sizeof(float))
            return { 0, { } };

        int SHOrder;
        switch(arr.shape[0])
        {
        case 1: SHOrder = 1; break;
        case 4: SHOrder = 2; break;
        case 9: SHOrder = 3; break;
        case 16: SHOrder = 4; break;
        case 25: SHOrder = 5; break;
        default: return { 0, { } };
        }

        int SHC = SHOrder * SHOrder;
        auto data = arr.data<float>();
        std::vector<Vec3f> coefs;

        if(arr.fortran_order)
        {
            // 有j行，SHC列
            coefs.resize(SHC);
            for(int j = 0; j < 3; ++j)
            {
                for(int i = 0; i < SHC; ++i)
                    coefs[i][j] = data[j * SHC + i];
            }
        }
        else
        {
            coefs.reserve(SHC);
            for(int i = 0, j = 0; i < SHC; ++i, j += 3)
            {
                Vec3f c(data[j], data[j + 1], data[j + 2]);
                coefs.push_back(c);
            }
        }

        return { SHOrder, std::move(coefs) };
    }
}

void NpySHToNormal(const std::string &npyFilename, int width, int height, const std::string &outputFilename)
{
    if(width < 0 || height < 0)
    {
        std::cout << "Invalid image size (width = " << width << ", height = " << height << ")" << std::endl;
        return;
    }

    auto [SHOrder, coefs] = LoadSHFromNPY(npyFilename);
    if(!SHOrder)
    {
        std::cout << "Failed to load SH coefs from " + npyFilename << std::endl;
        return;
    }

    auto SHTable = AGZ::Math::SH::GetSHTable<float>();
    int SHC = SHOrder * SHOrder;

    AGZ::Texture2D<Vec3f> image(width, height);
    for(uint32_t y = 0; y < image.GetHeight(); ++y)
    {
        float theta = AGZ::Math::PI<float> * (0.5f - (y + 0.5f) / image.GetHeight());
        float sinTheta = AGZ::Math::Sin(theta), cosTheta = AGZ::Math::Cos(theta);
        for(uint32_t x = 0; x < image.GetWidth(); ++x)
        {
            float phi = 2 * AGZ::Math::PI<float> * (x + 0.5f) / image.GetWidth();
            float sinPhi = AGZ::Math::Sin(phi), cosPhi = AGZ::Math::Cos(phi);

            float dirX = cosTheta * cosPhi;
            float dirZ = cosTheta * sinPhi;
            float dirY = sinTheta;
            Vec3f dir = Vec3f(dirX, dirY, dirZ).Normalize();

            Vec3f pixel;
            for(int i = 0; i < SHC; ++i)
                pixel += SHTable[i](dir) * coefs[i];
            image(x, y) = pixel;
        }
    }

    auto savedImage = image.Map([](const Vec3f &pixel)
    {
        return pixel.Map([](float c)
        {
            return uint8_t(AGZ::Math::Clamp<float>(c, 0, 1) * 255);
        });
    });
    AGZ::TextureFile::WriteRGBToPNG(outputFilename, savedImage);
}
