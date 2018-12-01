#include <iostream>

#include <Atrc/Atrc.h>

using namespace AGZ;
using namespace Atrc;
using namespace std;

const char *USAGE_MSG =
R"___(Usage:
    C2S [cube_texture_folder] [file_extension] spp outputSize workerCount [output_filename]
Example:
    C2S ./CubeTextures/ jpg 1000 256 4 ./Output.png)___";

bool LoadCubeTextures(const Str8 &foldername, const Str8 &ext, Texture2D<Spectrum> *texs)
{
    try
    {
        auto load = [](Texture2D<Spectrum> &tex, const FileSys::Path8 &filename)
        {
            tex = Texture2D<Spectrum>(
            TextureFile::LoadRGBFromFile(filename.ToStr()).Map(
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

struct Params
{
    Str8 cubeTexFolder;
    Str8 cubeTexExt;
    int spp             = 1;
    uint32_t outputSize = 256;
    int workerCount     = -1;
    Str8 outputFilename;
};

int C2S(const Params &params)
{
    Texture2D<Spectrum> cubeTexs[6];
    if(!LoadCubeTextures(params.cubeTexFolder, params.cubeTexExt, cubeTexs))
    {
        cout << "Failed to load cube environment textures from: "
             << params.cubeTexFolder.ToStdString() << endl;
        return -1;
    }
    
    Texture2D<Spectrum> output(params.outputSize, params.outputSize);

    Real xyUnit = Real(1) / params.outputSize;

    struct Rect { uint32_t xBegin, xEnd, yBegin, yEnd; };
    std::queue<Rect> tasks;

    for(uint32_t y = 0; y < params.outputSize; ++y)
        tasks.push({ 0, params.outputSize, y, y + 1 });
    
    StaticTaskDispatcher<Rect, NoSharedParam_t> dispatcher(params.workerCount);
    dispatcher.Run([&](const Rect &rect, NoSharedParam_t)
    {
        for(uint32_t y = rect.yBegin; y < rect.yEnd; ++y)
        {
            auto yBase = Real(y) / params.outputSize;
            for(uint32_t x = rect.xBegin; x < rect.xEnd; ++x)
            {
                Spectrum pixel;
                auto xBase = Real(x) / params.outputSize;
                for(int i = 0; i < params.spp; ++i)
                {
                    Real u = xBase + Math::Random::Uniform(Real(0), xyUnit);
                    Real v = yBase + Math::Random::Uniform(Real(0), xyUnit);
                    pixel += SampleCubeTextures(cubeTexs, SphereMapper<Real>::InvMap({ u, v }));
                }
                output(x, y) = pixel / params.spp;
            }
        }
    }, NO_SHARED_PARAM, tasks);

    TextureFile::WriteRGBToPNG(params.outputFilename, ToSavedImage(output));

    return 0;
}

int Run(int argc, char *argv[])
{
    if(argc != 7)
    {
        cout << USAGE_MSG << endl;
        return 0;
    }

    auto spp = Str8(argv[3]).Parse<int>();
    auto outputSize = Str8(argv[4]).Parse<uint32_t>();
    auto workerCount = Str8(argv[5]).Parse<int>();

    if(spp <= 0)
    {
        cout << "Invalid spp valid" << endl;
        return -1;
    }

    if(outputSize <= 0)
    {
        cout << "Invalid output size" << endl;
        return -1;
    }

    Params params;
    params.cubeTexFolder = argv[1];
    params.cubeTexExt = argv[2];
    params.spp = spp;
    params.outputSize = outputSize;
    params.workerCount = workerCount;
    params.outputFilename = argv[6];

    return C2S(params);
}

int main(int argc, char *argv[])
{
#ifndef _DEBUG
    try
    {
        return Run(argc, argv);
    }
    catch(const std::exception &err)
    {
        cout << err.what() << endl;
    }
    catch(...)
    {
        cout << "An unknown error occurred..." << endl;
    }

    return 0;
#else
    return Run(argc, argv);
#endif
}
