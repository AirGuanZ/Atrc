#include "../ParamParser.h"
#include "PathTracingIntegratorManager.h"

AGZ_NS_BEG(ObjMgr)

Atrc::PathTracingIntegrator *PureColorIntegratorCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto background = ParamParser::ParseSpectrum(params["background"]);
    auto entity     = ParamParser::ParseSpectrum(params["entity"]);
    return arena.Create<Atrc::PureColorIntegrator>(background, entity);
}

Atrc::PathTracingIntegrator *PathTracerCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto maxDepth = params["maxDepth"].AsValue().Parse<uint32_t>();
    return arena.Create<Atrc::PathTracer>(maxDepth);
}

Atrc::PathTracingIntegrator *VolumetricPathTracerCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto maxDepth = params["maxDepth"].AsValue().Parse<uint32_t>();
    return arena.Create<Atrc::VolumetricPathTracer>(maxDepth);
}

Atrc::PathTracingIntegrator *AmbientOcclusionIntegratorCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto maxOccuT = params["maxOccuT"].AsValue().Parse<Atrc::Real>();
    auto background = ParamParser::ParseSpectrum(params["background"]);
    auto object = ParamParser::ParseSpectrum(params["object"]);
    return arena.Create<Atrc::AmbientOcclusionIntegrator>(maxOccuT, background, object);
}

AGZ_NS_END(ObjMgr)
