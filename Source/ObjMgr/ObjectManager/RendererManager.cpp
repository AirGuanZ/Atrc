#include "RendererManager.h"

AGZ_NS_BEG(ObjMgr)

Atrc::ProgressReporter *DefaultProgressReporterCreator::Create([[maybe_unused]] const ConfigGroup &params, ObjArena<> &arena) const
{
    return arena.Create<Atrc::DefaultProgressReporter>();
}

Atrc::Renderer *ParallelRendererCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto workerCount = params["workerCount"].AsValue().Parse<int32_t>();
    auto spp         = params["spp"].AsValue().Parse<uint32_t>();
    auto integrator = GetSceneObject<Atrc::Integrator>(params["integrator"], arena);

    if(spp <= 0)
        throw SceneInitializationException("Invalid spp value");

    return arena.Create<Atrc::ParallelRenderer>(workerCount, spp, *integrator);
}

AGZ_NS_END(ObjMgr)
