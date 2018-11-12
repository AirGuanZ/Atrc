#pragma once

#include "../Common.h"

using LightCreator = ObjectCreator<Atrc::Light>;
using LightManager = ObjectManager<Atrc::Light>;

// direction = Vec3
// radiance = Spectrum
class DirectionalLightCreator : public LightCreator, public AGZ::Singleton<DirectionalLightCreator>
{
public:

	Str8 GetName() const override { return "DirectionalLight"; }

	Atrc::Light *Create(const ConfigGroup &params, ObjArena<> &arena) const override;
};

// top    = Spectrum
// bottom = Spectrum
class SkyLightCreator : public LightCreator, public AGZ::Singleton<SkyLightCreator>
{
public:

	Str8 GetName() const override { return "SkyLight"; }

	Atrc::Light *Create(const ConfigGroup &params, ObjArena<> &arena) const override;
};
