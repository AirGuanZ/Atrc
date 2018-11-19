#include "PostProcessorManager.h"

Atrc::PostProcessStage *ACESFilmCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    return arena.Create<Atrc::ACESFilm>();
}

Atrc::PostProcessStage *GammaCorrectorCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto gamma = params["gamma"].AsValue().Parse<float>();
    return arena.Create<Atrc::GammaCorrector>(gamma);
}
