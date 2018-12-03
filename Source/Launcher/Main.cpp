#include <iostream>

#include <Atrc/Atrc.h>
#include <ObjMgr/ObjectManager.h>

#include "SceneManager.h"

#if defined(_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>
#endif

using namespace AGZ;
using namespace Atrc;
using namespace std;

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

// 返回配置文件路径，为空时返回./Build/scene.txt，无法识别时返回None
Option<Str8> ParseParam(int argc, char *argv[])
{
    AGZ_ASSERT(argv);
    if(argc == 1)
        return "./Build/scene.txt";
    if(argc == 2)
    {
        if(argv[1] == Str8("-h") || argv[1] == Str8("--help"))
            return None;
        return argv[1];
    }
    return None;
}

int Run(const Str8 &sceneDescFilename)
{
    using namespace ObjMgr;

    Config config;
    if(!config.LoadFromFile(sceneDescFilename))
    {
        cout << "Failed to load scene configuration..." << endl;
        return -1;
    }
    auto &conf = config.Root();

    ObjArena<> arena;

    InitializeObjectManagers();
    InitializePublicObjects(conf, arena);

    auto &sceneMgr = SceneManager::GetInstance();

    auto filename = conf["outputFilename"].AsValue();
    auto width    = conf["camera.film.width"].AsValue().Parse<uint32_t>();
    auto height   = conf["camera.film.height"].AsValue().Parse<uint32_t>();

    sceneMgr.Initialize(config.Root());
    RenderTarget renderTarget(width, height);

    auto renderer        = GetSceneObject<Renderer>        (conf["renderer"],        arena);
    auto reporter        = GetSceneObject<ProgressReporter>(conf["reporter"],        arena);

    PostProcessor postProcessor;
    if(auto pps = conf.Find("postProcessors"))
        ObjMgr::AddPostProcessors(postProcessor, pps->AsArray(), arena);

    renderer->Render(sceneMgr.GetScene(), &renderTarget, reporter);

    postProcessor.Process(renderTarget);
    TextureFile::WriteRGBToPNG(filename, ToSavedImage(renderTarget));

    return 0;
}

int main(int argc, char *argv[])
{
#if defined(_MSC_VER) && defined(_DEBUG)
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG)
                 | _CRTDBG_LEAK_CHECK_DF);
#endif

    std::locale::global(std::locale(""));

    auto sceneDescFilename = ParseParam(argc, argv);
    if(!sceneDescFilename)
    {
        cout << "Usage:" << endl;
        cout << "    atrc [scene_desc]" << endl;
        cout << "    './Build/scene.txt' is used when [scene_desc] unspecified" << endl;
        return -1;
    }

#ifndef _DEBUG
    try
    {
        return Run(*sceneDescFilename);
    }
    catch(const std::exception &err)
    {
        cout << err.what() << endl;
    }
    catch(...)
    {
        cout << "An unknown error occurred..." << endl;
    }
#else
    return Run(*sceneDescFilename);
#endif
}
