#pragma once

#include <Atrc/Core/Common.h>
#include <Atrc/Core/Integrator.h>
#include <Atrc/Core/Scene.h>
#include <Atrc/Utility/GridDivider.h>

AGZ_NS_BEG(Atrc)

using SubareaRect = GridDivider<uint32_t>::Grid;

class SubareaRenderer
{
public:

    virtual ~SubareaRenderer() = default;

    virtual void Render(const Scene &scene, const Integrator &integrator, RenderTarget &rt, const SubareaRect &area) const = 0;
};

class ProgressReporter
{
public:

    virtual ~ProgressReporter() = default;

    virtual void Begin() = 0;

    virtual void End() = 0;

    virtual void Report(Real percent) = 0;

    virtual void Message(const Str8 &msg) = 0;
};

class Renderer
{
public:

    virtual ~Renderer() = default;

    virtual void Render(
        const SubareaRenderer &subareaRenderer, const Scene &scene, const Integrator &integrator,
        RenderTarget *rt, ProgressReporter *reporter) const = 0;
};

AGZ_NS_END(Atrc)
