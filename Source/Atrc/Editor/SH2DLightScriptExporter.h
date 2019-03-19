#pragma once

#include <Atrc/Editor/ResourceManagement/ResourceManager.h>
#include <Atrc/Editor/SceneExportingContext.h>

class SH2DLightScriptExporter
{
public:

    std::string Export(
        ResourceManager &rscMgr, SceneExportingContext &ctx, int SHOrder, int N) const;
};
