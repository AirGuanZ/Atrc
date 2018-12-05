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
// spp         = uint32_t
// gridSize    = uint32_t
// integrator  = IntegratorDefinition
class PathTracingRendererCreator : public RendererCreator, public AGZ::Singleton<PathTracingRendererCreator>
{
public:

    Str8 GetName() const override { return "PathTracingRenderer"; }

    Atrc::Renderer *Create(const ConfigGroup &params, ObjArena<> &arena) const override;
};

AGZ_NS_END(ObjMgr)
