#include "../GeometryManager/GeometryManager.h"
#include "../MaterialManager/MaterialManager.h"
#include "../ParamParser/ParamParser.h"
#include "EntityCreator.h"

Atrc::Entity *GeometricEntityCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto geometry = GetSceneObject<Atrc::Geometry>(params["geometry"], arena);
    auto material = GetSceneObject<Atrc::Material>(params["material"], arena);

	return arena.Create<Atrc::GeometricEntity>(
		geometry, material, Atrc::MediumInterface{ nullptr, nullptr });
}

Atrc::Entity *GeometricDiffuseLightCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto geometry = GetSceneObject<Atrc::Geometry>(params["geometry"], arena);
	auto radiance = ParamParser::ParseSpectrum(params["radiance"]);

	return arena.Create<Atrc::GeometricDiffuseLight>(
		Atrc::MediumInterface{ nullptr, nullptr }, geometry, radiance);
}
