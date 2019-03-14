#pragma once

#include <Atrc/Lib/Core/Common.h>

struct Scene2SHResult
{
    Atrc::Film *coefs;
    Atrc::Film *mask;
    Atrc::Film *albedo;
    Atrc::Film *normal;
};

void Scene2SH(
    int workerCount, int taskGridSize, int SHOrder,
    const Atrc::Scene &scene, Atrc::Sampler *sampler, Scene2SHResult *result);
