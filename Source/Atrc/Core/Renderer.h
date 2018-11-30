#pragma once

#include <Atrc/Core/Common.h>
#include <Atrc/Core/Integrator.h>
#include <Atrc/Core/Scene.h>
#include <Atrc/Utility/GridDivider.h>

AGZ_NS_BEG(Atrc)

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

    virtual void Render(const Scene &scene, RenderTarget *rt, ProgressReporter *reporter) const = 0;
};

AGZ_NS_END(Atrc)
