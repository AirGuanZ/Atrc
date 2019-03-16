#pragma once

#include <Atrc/Editor/ResourceManagement/ResourceManager.h>
#include <Atrc/Editor/SceneExportingContext.h>

class SH2DSceneScriptExporter
{
public:

    std::string Export(
        ResourceManager &rscMgr, SceneExportingContext &ctx,
        int workerCount, int taskGridSize, int SHOrder) const;
};
