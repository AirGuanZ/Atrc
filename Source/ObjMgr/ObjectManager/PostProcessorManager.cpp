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

Atrc::PostProcessStage *HorizontalFlipperCreator::Create([[maybe_unused]] const ConfigGroup &params, ObjArena<> &arena) const
{
    return arena.Create<Atrc::HorizontalFlipper>();
}

Atrc::PostProcessStage *VerticalFlipperCreator::Create([[maybe_unused]] const ConfigGroup &params, ObjArena<> &arena) const
{
    return arena.Create<Atrc::VerticalFlipper>();
}

void AddPostProcessors(Atrc::PostProcessor &processor, const ConfigArray &stageArr, ObjArena<> &arena)
{
    for(size_t i = 0; i < stageArr.Size(); ++i)
    {
        auto stage = GetSceneObject<Atrc::PostProcessStage>(stageArr[i], arena);
        if(!stage)
            throw SceneInitializationException("Invalid post processor stage");
        processor.AddStage(stage);
    }
}

AGZ_NS_END(ObjMgr)
