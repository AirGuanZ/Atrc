#pragma once

#include <Atrc/Mgr/Context.h>

namespace Atrc::Mgr
{

void RegisterBuiltinRendererCreators(Context &context);

/*
    type = PathTracing

    integrator   = PathTracingIntegrator
    workerCount  = int
    taskGridSize = int
*/
class PathTracingRendererCreator : public Creator<Renderer>
{
public:

    std::string GetTypeName() const override { return "PathTracing"; }

    Renderer *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

} // namespace Atrc::Mgr
