#pragma once

#include <Atrc/Editor/ResourceManagement/ResourceManager.h>
#include <Atrc/Editor/LauncherScriptExportingContext.h>

class LauncherScriptExporter
{
    ResourceManager &rscMgr_;
    LauncherScriptExportingContext &ctx_;

public:

    LauncherScriptExporter(ResourceManager &rscMgr, LauncherScriptExportingContext &ctx);

    std::string Export() const;
};
