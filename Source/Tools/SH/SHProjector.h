#pragma once

#include <Atrc/Core/Core.h>
#include <Atrc/Utility/GridDivider.h>

using SubareaRect = Atrc::GridDivider<uint32_t>::Grid;

class SHEntityProjector
{
public:

    static void Project(
        const Atrc::Ray &r, const Atrc::Scene &scene, int SHC, int N,
        Atrc::Spectrum *output, AGZ::ObjArena<> &arena);
};

class SHLightProjector
{
public:

    static void Project(const Atrc::Light *light, int SHC, int N, Atrc::Spectrum *output);
};

class SHEntitySubareaRenderer
{
    int spp_, SHC_, N_;

public:

    explicit SHEntitySubareaRenderer(int spp, int SHC, int N);

    void Render(
        const Atrc::Scene &scene,
        Atrc::RenderTarget *renderTarget, const SubareaRect &area) const;
};

class SHEntityRenderer
{
    int workerCount_;

public:

    explicit SHEntityRenderer(int workerCount);

    void Render(
        const SHEntitySubareaRenderer &subareaRenderer, const Atrc::Scene &scene,
        Atrc::RenderTarget *output, Atrc::ProgressReporter *reporter) const;
};
