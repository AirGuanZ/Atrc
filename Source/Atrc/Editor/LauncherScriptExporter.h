#pragma once

#include <Atrc/Editor/ResourceManagement/ResourceManager.h>
#include <Atrc/Editor/EditorCore.h>
#include <Atrc/Editor/SceneExportingContext.h>

class LauncherScriptExporter
{
    ResourceManager &rscMgr_;
    SceneExportingContext &ctx_;

public:

    LauncherScriptExporter(ResourceManager &rscMgr, SceneExportingContext &ctx);

    std::string Export(const RendererInstance *renderer, const std::string &outputFilename) const;
};

class LauncherScriptImporter
{
public:

    void Import(const AGZ::ConfigGroup &root, EditorData *data, std::string_view scriptFilename);
};
