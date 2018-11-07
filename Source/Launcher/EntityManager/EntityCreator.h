#pragma once

#include "../Common.h"

using EntityCreator = ObjectCreator<Atrc::Entity>;
using EntityManager = ObjectManager<Atrc::Entity>;

// geometry = GeometryDefinition
// material = MaterialDefinition
// TODO: medium
class GeometricEntityCreator : public EntityCreator, public AGZ::Singleton<GeometricEntityCreator>
{
public:

	Str8 GetName() const override { return "GeometricEntity"; }

	Atrc::Entity *Create(const ConfigGroup &params, ObjArena<> &arena) const override;
};

// geometry = GeometryDefinition
// radiance = Spectrum
// TODO: medium
class GeometricDiffuseLightCreator : public EntityCreator, public AGZ::Singleton<GeometricDiffuseLightCreator>
{
public:

	Str8 GetName() const override { return "GeometricDiffuseLight"; }

	Atrc::Entity *Create(const ConfigGroup &params, ObjArena<> &arena) const override;
};
