#pragma once

#include <Atrc/Core/Core.h>

class SHEntityProjector
{
public:

    static void Project(
        const Atrc::Ray &r, const Atrc::Scene &scene, int N,
        Atrc::Spectrum (&output)[9], AGZ::ObjArena<> &arena);
};

class SHLightProjector
{
public:

    static void Project(const Atrc::Light *light, int N, Atrc::Spectrum(&output)[9]);
};

class SHEntitySubareaRenderer
{
    int spp_, N_;

public:

    explicit SHEntitySubareaRenderer(int spp, int N);

    void Render(
        const Atrc::Scene &scene,
        Atrc::RenderTarget *renderTarget, const Atrc::SubareaRect &area) const;
};

class SHEntityRenderer
{
    int workerCount_;

public:

    explicit SHEntityRenderer(int workerCount = -1);

    void Render(
        const SHEntitySubareaRenderer &subareaRenderer, const Atrc::Scene &scene,
        Atrc::RenderTarget *output, Atrc::ProgressReporter *reporter) const;
};
