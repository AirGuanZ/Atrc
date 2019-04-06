#pragma once

#include <Atrc/Editor/Light/Light.h>
#include <Atrc/Editor/ResourceManagement/ResourceManager.h>
#include <Atrc/Editor/SceneExportingContext.h>

class SH2DLightScriptExporter
{
public:

    std::string Export(
        const ILight *light, SceneExportingContext &ctx, int SHOrder, int N) const;
};
