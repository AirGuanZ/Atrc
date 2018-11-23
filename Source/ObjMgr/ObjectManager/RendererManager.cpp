#include "RendererManager.h"

AGZ_NS_BEG(ObjMgr)

Atrc::ProgressReporter *DefaultProgressReporterCreator::Create([[maybe_unused]] const ConfigGroup &params, ObjArena<> &arena) const
{
    return arena.Create<Atrc::DefaultProgressReporter>();
}

Atrc::Renderer *ParallelRendererCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto workerCount = params["workerCount"].AsValue().Parse<int32_t>();
    return arena.Create<Atrc::ParallelRenderer>(workerCount);
}

Atrc::Renderer *SerialRendererCreator::Create([[maybe_unused]] const ConfigGroup &params, ObjArena<> &arena) const
{
    return arena.Create<Atrc::SerialRenderer>();
}

Atrc::SubareaRenderer *JitteredSubareaRendererCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto spp = params["spp"].AsValue().Parse<uint32_t>();
    if(spp < 1)
        throw SceneInitializationException("JitteredSubareaRendererCreator: invalid spp");
    return arena.Create<Atrc::JitteredSubareaRenderer>(spp);
}

AGZ_NS_END(ObjMgr)
