#pragma once

#include <filesystem>

#include <AGZUtils/Utils/Config.h>

struct ProjectLightOutputArgs
{
    std::filesystem::path dir;
    bool outputTXT;
    bool outputNPZ;
};

void ProjectLight(const AGZ::Config &config, const std::string &configPath, const ProjectLightOutputArgs &outputArgs);
