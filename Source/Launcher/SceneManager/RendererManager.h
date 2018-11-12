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

using SubareaRendererCreator = ObjectCreator<Atrc::SubareaRenderer>;
using SubareaRendererManager = ObjectManager<Atrc::SubareaRenderer>;

// spp = uint32_t
class JitteredSubareaRendererCreator : public SubareaRendererCreator, public AGZ::Singleton<JitteredSubareaRendererCreator>
{
public:

    Str8 GetName() const override { return "JitteredSubareaRenderer"; }

    Atrc::SubareaRenderer *Create(const ConfigGroup &params, ObjArena<> &arena) const override;
};
