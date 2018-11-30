#pragma once

#include "../Common.h"

AGZ_NS_BEG(ObjMgr)

using ProgressReporterCreator = ObjectCreator<Atrc::ProgressReporter>;
using ProgressReporterManager = ObjectManager<Atrc::ProgressReporter>;

class DefaultProgressReporterCreator : public ProgressReporterCreator, public AGZ::Singleton<DefaultProgressReporterCreator>
{
public:

    Str8 GetName() const override { return "Default"; }

    Atrc::ProgressReporter *Create(const ConfigGroup &params, ObjArena<> &arena) const override;
};

using RendererCreator = ObjectCreator<Atrc::Renderer>;
using RendererManager = ObjectManager<Atrc::Renderer>;

// workerCount = int32_t
// spp_        = int32_t
// integrator  = IntegratorDefinition
class ParallelRendererCreator : public RendererCreator, public AGZ::Singleton<ParallelRendererCreator>
{
public:

    Str8 GetName() const override { return "ParallelRenderer"; }

    Atrc::Renderer *Create(const ConfigGroup &params, ObjArena<> &arena) const override;
};

AGZ_NS_END(ObjMgr)
