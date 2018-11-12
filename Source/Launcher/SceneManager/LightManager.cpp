#include "../ParamParser/ParamParser.h"
#include "LightManager.h"

Atrc::Light *DirectionalLightCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
	auto direction = ParamParser::ParseVec3(params["direction"]);
	auto radiance = ParamParser::ParseSpectrum(params["radiance"]);

	return arena.Create<Atrc::DirectionalLight>(direction, radiance);
}

Atrc::Light *SkyLightCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
	auto top    = ParamParser::ParseSpectrum(params["top"]);
	auto bottom = ParamParser::ParseSpectrum(params["bottom"]);

	return arena.Create<Atrc::SkyLight>(top, bottom);
}
