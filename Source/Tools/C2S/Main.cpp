#include <iostream>

#include <Atrc/Atrc.h>

using namespace AGZ;
using namespace Atrc;
using namespace std;

const char *USAGE_MSG =
R"___(Usage:
    C2S [cube_texture_folder] [file_extension] spp sidelen [output_filename]
Example:
    C2S ./CubeTextures/ jpg 1000 256 ./Output.png)___";

bool LoadCubeTextures(const Str8 &foldername, const Str8 &ext, Texture2D<Spectrum> *texs)
{
    try
    {
        auto load = [](Texture2D<Spectrum> &tex, const FileSys::Path8 &filename)
        {
            tex = Texture2D<Spectrum>(
            TextureFile::LoadRGBFromFile(filename.ToStr().ToStdWString()).Map(
                [](const auto &c) { return c.Map([](uint8_t b) { return b / 255.0f; }); }));
        };

        FileSys::Path8 path(foldername, false);

        load(texs[0], path.SetFilename("front").SetExtension(ext));
        load(texs[1], path.SetFilename("left") .SetExtension(ext));
        load(texs[2], path.SetFilename("up")   .SetExtension(ext));
        load(texs[3], path.SetFilename("back") .SetExtension(ext));
        load(texs[4], path.SetFilename("right").SetExtension(ext));
        load(texs[5], path.SetFilename("down") .SetExtension(ext));

        return true;
    }
    catch(...)
    {
        return false;
    }
}

Spectrum SampleCubeTextures(const Texture2D<Spectrum> *texs, const Vec3 &dir)
{
    auto coord = CubeMapper<Real>::Map(dir);
    return AGZ::LinearSampler::Sample(texs[static_cast<int>(coord.face)], Vec2(coord.uv.u, 1 - coord.uv.v));
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

int main(int argc, char *argv[])
{
    if(argc != 6)
    {
        cout << USAGE_MSG << endl;
        return 0;
    }

    Texture2D<Spectrum> cubeTexs[6];
    if(!LoadCubeTextures(argv[1], argv[2], cubeTexs))
    {
        cout << "Failed to load cube environment textures from: " << argv[1] << endl;
        return -1;
    }

    int spp = Str8(argv[3]).Parse<int>();
    if(spp <= 0)
    {
        cout << "Invalid spp valid" << endl;
        return -1;
    }

    uint32_t sidelen = Str8(argv[4]).Parse<uint32_t>();
    Texture2D<Spectrum> output(sidelen, sidelen);

    Real xyUnit = Real(1) / sidelen;

    for(uint32_t y = 0; y < sidelen; ++y)
    {
        auto yBase = Real(y) / sidelen;
        for(uint32_t x = 0; x < sidelen; ++x)
        {
            Spectrum pixel;
            auto xBase = Real(x) / sidelen;
            for(int i = 0; i < spp; ++i)
            {
                Real u = xBase + Math::Random::Uniform(Real(0), xyUnit);
                Real v = yBase + Math::Random::Uniform(Real(0), xyUnit);
                pixel += SampleCubeTextures(cubeTexs, SphereMapper<Real>::InvMap({ u, v }));
            }
            output(x, y) = pixel / spp;
        }
    }

    TextureFile::WriteRGBToPNG(argv[5], ToSavedImage(output));

    return 0;
}
