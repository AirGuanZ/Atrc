#pragma once

#include <Atrc/Editor/Camera.h>
#include <Atrc/Editor/EntityController.h>
#include <Atrc/Editor/ScriptBuilder.h>

class CameraInstance;
class FilmFilterInstance;
class RendererInstance;
class SamplerInstance;

class SceneExportingContext : public ScriptBuilder
{
public:

    const std::filesystem::path workspaceDirectory;
    const std::filesystem::path scriptDirectory;

    const EntityController *entityController;

    const DefaultRenderingCamera * const activeCamera;
    const CameraInstance     * const camera;

    float filmAspectRatio;

    SceneExportingContext(
        std::filesystem::path workspace, std::filesystem::path scriptDir,
        const DefaultRenderingCamera *activeCamera,
        const CameraInstance *camera,
        float filmAspectRatio)
        : workspaceDirectory(std::move(workspace)), scriptDirectory(std::move(scriptDir)),
          entityController(nullptr),
          activeCamera(activeCamera), camera(camera), filmAspectRatio(filmAspectRatio)
    {

    }
};
