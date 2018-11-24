#pragma once

#include <Atrc/Core/Core.h>

class SHProjector
{
public:

    virtual ~SHProjector() = default;

    virtual void Project(
        const Atrc::Ray &r, const Atrc::Scene &scene, int N,
        Atrc::Spectrum(&output)[9], AGZ::ObjArena<> &arena) const = 0;
};

class SHEntityProjector : public SHProjector
{
public:

    void Project(
        const Atrc::Ray &r, const Atrc::Scene &scene, int N,
        Atrc::Spectrum (&output)[9], AGZ::ObjArena<> &arena) const override;
};

class SHLightProjector : public SHProjector
{
public:

    void Project(
        const Atrc::Ray &r, const Atrc::Scene &scene, int N,
        Atrc::Spectrum(&output)[9], AGZ::ObjArena<> &arena) const override;
};

class SHSubareaRenderer
{
    int spp_, N_;

public:

    explicit SHSubareaRenderer(int spp, int N);

    void Render(
        const Atrc::Scene &scene, const SHProjector &projector,
        Atrc::RenderTarget *renderTarget, const Atrc::SubareaRect &area) const;
};

class SHRenderer
{
    int workerCount_;

public:

    explicit SHRenderer(int workerCount = -1);

    void Render(
        const SHSubareaRenderer &subareaRenderer, const Atrc::Scene &scene, const SHProjector &projector,
        Atrc::RenderTarget *output, Atrc::ProgressReporter *reporter) const;
};
