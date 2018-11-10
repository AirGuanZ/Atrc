#pragma once

#include "../Common.h"

using RendererCreator = ObjectCreator<Atrc::Renderer>;
using RendererManager = ObjectManager<Atrc::Renderer>;

// workerCount = int32_t
class ParallelRendererCreator : public RendererCreator, public AGZ::Singleton<ParallelRendererCreator>
{
public:

    Str8 GetName() const override { return "ParallelRenderer"; }

    Atrc::Renderer *Create(const ConfigGroup &params, ObjArena<> &arena) const override;
};

// no param
class SerialRendererCreator : public RendererCreator, public AGZ::Singleton<SerialRendererCreator>
{
public:

    Str8 GetName() const override { return "SerialRenderer"; }

    Atrc::Renderer *Create(const ConfigGroup &params, ObjArena<> &arena) const override;
};
