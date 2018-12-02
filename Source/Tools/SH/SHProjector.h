#pragma once

#include <Atrc/Core/Core.h>
#include <Atrc/Utility/GridDivider.h>

using SubareaRect = Atrc::GridDivider<uint32_t>::Grid;

class SHEntityProjector
{
public:

    virtual ~SHEntityProjector() = default;

    virtual void Project(
        const Atrc::Ray &r, const Atrc::Scene &scene, int SHC, int N,
        Atrc::Spectrum *output, AGZ::ObjArena<> &arena) const = 0;
};

class SHEntityDirectProjector : public SHEntityProjector
{
public:

    void Project(
        const Atrc::Ray &r, const Atrc::Scene &scene, int SHC, int N,
        Atrc::Spectrum *output, AGZ::ObjArena<> &arena) const override;
};

class SHEntityFullProjector : public SHEntityProjector
{
    int maxDepth_;

    void ProjectImpl(const Atrc::Ray &r, const Atrc::Scene &scene, int SHC,
                     Atrc::Spectrum *output, AGZ::ObjArena<> &arena, int depth) const;

public:

    explicit SHEntityFullProjector(int maxDepth);

    void Project(const Atrc::Ray &r, const Atrc::Scene &scene, int SHC, int N,
                 Atrc::Spectrum *output, AGZ::ObjArena<> &arena) const override;
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
        const Atrc::Scene &scene, const SHEntityProjector &projector,
        Atrc::RenderTarget *renderTarget, const SubareaRect &area) const;
};

class SHEntityRenderer
{
    int workerCount_;

public:

    explicit SHEntityRenderer(int workerCount);

    void Render(
        const SHEntitySubareaRenderer &subareaRenderer, const Atrc::Scene &scene, const SHEntityProjector &projector,
        Atrc::RenderTarget *output, Atrc::ProgressReporter *reporter) const;
};
