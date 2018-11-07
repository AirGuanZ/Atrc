#include "../../ParamParser/ParamParser.h"
#include "FresnelCreator.h"
#include "IdealMirrorCreator.h"

const Atrc::Material *IdealMirrorCreator::Create(MaterialManager &matMgr, const ConfigGroup &params, ObjArena<> &arena) const
{
	auto rc = ParamParser::ParseSpectrum(params["rc"]);

	auto &fresnelTypeName = params["fresnel.type"].AsValue();
	auto fresnelType = FresnelCreator::Name2Type(fresnelTypeName);
	auto fresnel = FresnelCreator::CreateFresnel(fresnelType, params["fresnel"].AsGroup(), arena);

	return arena.Create<Atrc::IdealMirror>(rc, fresnel);
}
