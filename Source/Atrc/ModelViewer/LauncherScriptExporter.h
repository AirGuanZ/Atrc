#pragma once

#include <Atrc/ModelViewer/ResourceManagement/ResourceManager.h>
#include <Atrc/ModelViewer/LauncherScriptExportingContext.h>

class LauncherScriptExporter
{
    ResourceManager &rscMgr_;
    LauncherScriptExportingContext &ctx_;

public:

    LauncherScriptExporter(ResourceManager &rscMgr, LauncherScriptExportingContext &ctx);

    std::string Export() const;
};
