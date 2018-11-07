#pragma once

#include "../Common.h"

class ParamParsingError : public std::runtime_error
{
public:

	explicit ParamParsingError(const std::string &str)
		: std::runtime_error(str.c_str())
	{
		
	}
};

class ParamParser
{
public:

	// 0.1				=> (0.1f, 0.1f, 0.1f)
	// ()				=> (0.0f, 0.0f, 0.0f)
	// (0.1)			=> (0.1f, 0.1f, 0.1f)
	// (0.1, 0.2, 0.3)	=> (0.1f, 0.2f, 0.3f)
	static Atrc::Spectrum ParseSpectrum(const ConfigNode &node);

	// (0.1, 0.2, 0.3) => (0.1, 0.2, 0.3)
	static Atrc::Vec3 ParseVec3(const ConfigNode &node);

	// () => identity
	// ((Scale, 5.0), (Translate, 0.1, 0.6, 0.7), (RotateX, Deg, 47))
	static Atrc::Transform ParseTransform(const ConfigNode &node);
};
