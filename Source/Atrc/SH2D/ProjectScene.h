#pragma once

#include <filesystem>

#include <AGZUtils/Utils/Config.h>

struct ProjectSceneOutputArgs
{
    std::filesystem::path dir;
    bool outputHDR;
    bool outputSH2D;
    bool outputNPZ;
};

void ProjectScene(const AGZ::Config &config, const std::string &configPath, const ProjectSceneOutputArgs &outputArgs);
