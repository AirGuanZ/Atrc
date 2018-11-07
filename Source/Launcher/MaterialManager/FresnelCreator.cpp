#include "../../ParamParser/ParamParser.h"
#include "FresnelCreator.h"

FresnelType FresnelCreator::Name2Type(const Str8 &name)
{
	if(name == "FresnelConductor")
		return FresnelType::FresnelConductor;
	if(name == "FresnelDielectric")
		return FresnelType::FresnelDielectric;
	if(name == "SchlickApproximation")
		return FresnelType::SchlickApproximation;
	throw std::invalid_argument("FresnelCreator: unknown fresnel type");
}

const Atrc::Fresnel *FresnelCreator::CreateFresnel(const FresnelType &type, const ConfigGroup &params, ObjArena<>& arena)
{
	switch(type)
	{
	case FresnelType::FresnelConductor:
		return CreateFresnelConductor(params, arena);
	default:
		return CreateDielectric(type, params, arena);
	}
}

const Atrc::Dielectric *FresnelCreator::CreateDielectric(const FresnelType &type, const ConfigGroup &params, ObjArena<> &arena)
{
	switch(type)
	{
	case FresnelType::FresnelDielectric:
		return CreateFresnelDielectric(params, arena);
	case FresnelType::SchlickApproximation:
		return CreateSchlickApproximation(params, arena);
	default:
		std::terminate();
	}
}

const Atrc::FresnelConductor *FresnelCreator::CreateFresnelConductor(const ConfigGroup &params, ObjArena<> &arena)
{
	auto etaI = ParamParser::ParseSpectrum(params["etaI"]);
	auto etaT = ParamParser::ParseSpectrum(params["etaT"]);
	auto k    = ParamParser::ParseSpectrum(params["k"]);
	return arena.Create<Atrc::FresnelConductor>(etaI, etaT, k);
}

const Atrc::FresnelDielectric *FresnelCreator::CreateFresnelDielectric(const ConfigGroup &params, ObjArena<> &arena)
{
	auto etaI = params["etaI"].AsValue().Parse<float>();
	auto etaT = params["etaT"].AsValue().Parse<float>();
	return arena.Create<Atrc::FresnelDielectric>(etaI, etaT);
}

const Atrc::SchlickApproximation *FresnelCreator::CreateSchlickApproximation(const ConfigGroup &params, ObjArena<> &arena)
{
	auto etaI = params["etaI"].AsValue().Parse<float>();
	auto etaT = params["etaT"].AsValue().Parse<float>();
	return arena.Create<Atrc::SchlickApproximation>(etaI, etaT);
}
