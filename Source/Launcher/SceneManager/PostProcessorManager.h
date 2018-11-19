#pragma once

#include "../Common.h"

using PostProcessorStageCreator = ObjectCreator<Atrc::PostProcessStage>;
using PostProcessorStageManager = ObjectManager<Atrc::PostProcessStage>;

class ACESFilmCreator : public PostProcessorStageCreator, public AGZ::Singleton<ACESFilmCreator>
{
public:

    Str8 GetName() const override { return "ACESFilm"; }

    Atrc::PostProcessStage* Create(const ConfigGroup &params, ObjArena<> &arena) const override;
};

class GammaCorrectorCreator : public PostProcessorStageCreator, public AGZ::Singleton<GammaCorrectorCreator>
{
public:

    Str8 GetName() const override { return "GammaCorrection"; }

    Atrc::PostProcessStage *Create(const ConfigGroup &params, ObjArena<> &arena) const override;
};
