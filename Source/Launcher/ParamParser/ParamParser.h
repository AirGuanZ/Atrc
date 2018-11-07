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
};
