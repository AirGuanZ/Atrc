#pragma once

#include <string>

void ReconstructImage(
    int SHOrder, const std::string &lightFilename,
    const std::string *sceneCoefFilenames,
    const std::string &albedoFilename,
    const std::string &outputFilename,
    float rotateDeg);
