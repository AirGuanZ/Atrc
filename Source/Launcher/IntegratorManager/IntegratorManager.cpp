#include "../ParamParser/ParamParser.h"
#include "IntegratorManager.h"

Atrc::Integrator *PureColorIntegratorCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto background = ParamParser::ParseSpectrum(params["background"]);
    auto entity     = ParamParser::ParseSpectrum(params["entity"]);
    return arena.Create<Atrc::PureColorIntegrator>(background, entity);
}

Atrc::Integrator *PathTracerCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto maxDepth = params["maxDepth"].AsValue().Parse<uint32_t>();
    return arena.Create<Atrc::PathTracer>(maxDepth);
}

Atrc::Integrator *VolumetricPathTracerCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto maxDepth = params["maxDepth"].AsValue().Parse<uint32_t>();
    return arena.Create<Atrc::VolumetricPathTracer>(maxDepth);
}
