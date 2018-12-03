#pragma once

#include "../Common.h"

AGZ_NS_BEG(ObjMgr)

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

class HorizontalFlipperCreator : public PostProcessorStageCreator, public AGZ::Singleton<HorizontalFlipperCreator>
{
public:

    Str8 GetName() const override { return "HorizontalFlipper"; }

    Atrc::PostProcessStage *Create(const ConfigGroup &params, ObjArena<> &arena) const override;
};

class VerticalFlipperCreator : public PostProcessorStageCreator, public AGZ::Singleton<VerticalFlipperCreator>
{
public:

    Str8 GetName() const override { return "VerticalFlipper"; }

    Atrc::PostProcessStage *Create(const ConfigGroup &params, ObjArena<> &arena) const override;
};

void AddPostProcessors(Atrc::PostProcessor &processor, const ConfigArray &stageArr, ObjArena<> &arena);

AGZ_NS_END(ObjMgr)
