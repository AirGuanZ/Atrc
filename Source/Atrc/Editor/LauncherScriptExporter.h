#pragma once

#include <Atrc/Editor/ResourceManagement/ResourceManager.h>
#include <Atrc/Editor/EditorCore.h>
#include <Atrc/Editor/SceneExportingContext.h>

#include <Atrc/Editor/FilmFilter/FilmFilter.h>

class LauncherScriptExporter
{
public:

    std::string Export(
        ResourceManager &rscMgr, SceneExportingContext &ctx,
        const RendererInstance *renderer,
        const IFilmFilter *filmFilter,
        const SamplerInstance    *sampler,
        const Vec2i &outputFilmSize,
        const std::string &outputFilename) const;
};

class LauncherScriptImporter
{
public:

    void Import(const AGZ::ConfigGroup &root, EditorData *data, std::string_view scriptFilename);
};
