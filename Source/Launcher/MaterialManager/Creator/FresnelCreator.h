#pragma once

#include "../../Common.h"

enum class FresnelType
{
	FresnelConductor,	 // "FresnelConductor"
	FresnelDielectric,   // "FresnelDielectric"
	SchlickApproximation // "SchlickApproximation"
};

class FresnelCreator
{
public:

	static Option<FresnelType> Name2Type(const Str8 &name);

	static const Atrc::Fresnel *CreateFresnel(const FresnelType &type, const ConfigGroup &params, ObjArena<> &arena);

	static const Atrc::Dielectric *CreateDielectric(const FresnelType &type, const ConfigGroup &params, ObjArena<> &arena);

	// etaI = Spectrum;
	// etaT = Spectrum;
	// k	= Spectrum;
	static const Atrc::FresnelConductor *CreateFresnelConductor(const ConfigGroup &params, ObjArena<> &arena);

	// etaI = float
	// etaT = float
	static const Atrc::FresnelDielectric *CreateFresnelDielectric(const ConfigGroup &params, ObjArena<> &arena);

	// etaI = float
	// etaT = float
	static const Atrc::SchlickApproximation *CreateSchlickApproximation(const ConfigGroup &params, ObjArena<> &arena);
};
