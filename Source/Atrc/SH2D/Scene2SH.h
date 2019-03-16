#pragma once

#include <Atrc/Lib/Core/Common.h>

struct Scene2SHResult
{
    Atrc::Film *coefs  = nullptr;
    Atrc::Film *binary = nullptr;
    Atrc::Film *albedo = nullptr;
    Atrc::Film *normal = nullptr;
};

void Scene2SH(
    int workerCount, int taskGridSize, int SHOrder,
    const Atrc::Scene &scene, Atrc::Sampler *sampler, Scene2SHResult *result);
