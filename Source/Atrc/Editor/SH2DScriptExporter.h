#pragma once

#include <Atrc/Editor/ResourceManagement/ResourceManager.h>
#include <Atrc/Editor/SceneExportingContext.h>

#include <Atrc/Editor/Sampler/Sampler.h>

#include <Atrc/Editor/FilmFilter/FilmFilter.h>

class SH2DSceneScriptExporter
{
public:

    std::string Export(
        ResourceManager &rscMgr, SceneExportingContext &ctx,
        int workerCount, int taskGridSize, int SHOrder,
        const Vec2i &outputFilmSize,
        const Atrc::Editor::IFilmFilter *filmFilter,
        const Atrc::Editor::ISampler *sampler) const;
};
