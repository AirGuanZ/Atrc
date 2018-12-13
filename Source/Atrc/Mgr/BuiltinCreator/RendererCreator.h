#pragma once

#include <Atrc/Mgr/Context.h>

namespace Atrc::Mgr
{

/*
    type = PathTracing

    integrator   = PathTracingIntegrator
    workerCount  = int
    taskGridSize = int
*/
class PathTracingRendererCreator : public Creator<Renderer>
{
public:

    Str8 GetTypeName() const override { return "PathTracing"; }

    const Renderer *Create(const ConfigGroup &group, Context &context, Arena &arena) override;
};

} // namespace Atrc::Mgr
