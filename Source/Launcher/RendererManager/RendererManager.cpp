#include "RendererManager.h"

Atrc::Renderer *ParallelRendererCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto workerCount = params["workerCount"].AsValue().Parse<int32_t>();
    return arena.Create<Atrc::ParallelRenderer>(workerCount);
}

Atrc::Renderer *SerialRendererCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    return arena.Create<Atrc::SerialRenderer>();
}
