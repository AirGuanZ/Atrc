#include "../../ParamParser/ParamParser.h"
#include "PlasticCreator.h"

const Atrc::Material *PlasticCreator::Create(MaterialManager &matMgr, const ConfigGroup &params, ObjArena<> &arena) const
{
	auto kd = ParamParser::ParseSpectrum(params["kd"]);
	auto ks = ParamParser::ParseSpectrum(params["ks"]);
	auto roughness = params["roughness"].AsValue().Parse<Atrc::Real>();

	return arena.Create<Atrc::Plastic>(kd, ks, roughness);
}
