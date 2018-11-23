#include "PostProcessorManager.h"

AGZ_NS_BEG(ObjMgr)

Atrc::PostProcessStage *ACESFilmCreator::Create([[maybe_unused]] const ConfigGroup &params, ObjArena<> &arena) const
{
    return arena.Create<Atrc::ACESFilm>();
}

Atrc::PostProcessStage *GammaCorrectorCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto gamma = params["gamma"].AsValue().Parse<float>();
    return arena.Create<Atrc::GammaCorrector>(gamma);
}

AGZ_NS_END(ObjMgr)
