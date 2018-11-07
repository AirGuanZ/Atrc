#pragma once

#include "../Common.h"

class MaterialCreator
{
public:

	virtual ~MaterialCreator() = default;

	virtual const Atrc::Material *Create(const ConfigGroup &params, ObjArena<> &arena) const = 0;
};

//================================= Fresnel Creator =================================

enum class FresnelType
{
	FresnelConductor,	 // "FresnelConductor"
	FresnelDielectric,   // "FresnelDielectric"
	SchlickApproximation // "SchlickApproximation"
};

class FresnelCreator
{
public:

	static FresnelType Name2Type(const Str8 &name);

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

#define SIMPLE_MATERIAL_CREATOR(MAT_NAME) \
	class MAT_NAME##Creator : public MaterialCreator \
	{ \
	public: \
		const Atrc::Material *Create(const ConfigGroup &params, ObjArena<> &arena) const override; \
	}

// No param
SIMPLE_MATERIAL_CREATOR(BlackMaterial);

// albedo = Spectrum
SIMPLE_MATERIAL_CREATOR(DiffuseMaterial);

// rc = Spectrum
// fresnel = {
//		type = DielectricType
//		...fresnel params
// }
SIMPLE_MATERIAL_CREATOR(FresnelSpecular);

// rc = Spectrum
// fresnel = {
//		type = FresnelType
//		...fresnel params
// }
SIMPLE_MATERIAL_CREATOR(IdealMirror);

// rc        = Spectrum
// etaI      = Spectrum
// etaT      = Spectrum
// k         = Spectrum
// roughness = Real
SIMPLE_MATERIAL_CREATOR(Metal);

// kd = Spectrum
// ks = Spectrum
// roughness = Real
SIMPLE_MATERIAL_CREATOR(Plastic);

// sampler = Linear/Nearest
// texture = "filename"
// internal = MaterialDefinition
SIMPLE_MATERIAL_CREATOR(TextureScaler);

// No param
SIMPLE_MATERIAL_CREATOR(UncallableMaterial);

#undef SIMPLE_MATERIAL_CREATOR
