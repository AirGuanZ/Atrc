#pragma once

#include <sstream>

#include <Atrc/Editor/Camera.h>
#include <Atrc/Editor/EntityController.h>

class CameraInstance;
class FilmFilterInstance;
class RendererInstance;
class SamplerInstance;

class LauncherScriptExportingContext
{
    size_t indent_;
    std::string indentStr_;

    std::stringstream sst_;

    void AddLineAux() const { }

    template<typename TStr>
    void AddLineAux(TStr &&str)
    {
        sst_ << std::forward<TStr>(str);
    }

    template<typename TStr0, typename TStr1, typename...Others>
    void AddLineAux(TStr0 &&str0, TStr1 &&str1, Others&&...others)
    {
        AddLineAux(std::forward<TStr0>(str0));
        AddLineAux(std::forward<TStr1>(str1), std::forward<Others>(others)...);
    }

public:

    const DefaultRenderingCamera * const activeCamera;

    const CameraInstance * const camera;
    const FilmFilterInstance * const filmFilter;
    const RendererInstance * const renderer;
    const SamplerInstance * const sampler;

    const std::string workspaceDirectory;
    const std::string scriptDirectory;
    const std::string outputFilename;

    Vec2i outputFilmSize;

    const EntityController *entityController;

    LauncherScriptExportingContext(
        const DefaultRenderingCamera *activeCamera,
        const CameraInstance *camera,
        const FilmFilterInstance *filmFilter,
        const RendererInstance *renderer,
        const SamplerInstance *sampler,
        std::string workspace, std::string scriptDir, std::string outputFilename,
        const Vec2i &outputFilmSize)
        : indent_(0),
          activeCamera(activeCamera),
          camera(camera),
          filmFilter(filmFilter),
          renderer(renderer),
          sampler(sampler),
          workspaceDirectory(std::move(workspace)), scriptDirectory(std::move(scriptDir)),
          outputFilename(std::move(outputFilename)),
          outputFilmSize(outputFilmSize),
          entityController(nullptr)
    {
        AGZ_ASSERT(activeCamera);
    }

    void IncIndent()
    {
        indentStr_ = std::string(4 * ++indent_, ' ');
    }

    void DecIndent()
    {
        indentStr_ = std::string(4 * --indent_, ' ');
    }

    std::string GetString() const
    {
        return sst_.str();
    }

    template<typename...TStrs>
    void AddLine(TStrs&&...strs)
    {
        sst_ << indentStr_;
        AddLineAux(std::forward<TStrs>(strs)...);
        sst_ << std::endl;
    }

    void ClearString()
    {
        sst_.str("");
    }
};
