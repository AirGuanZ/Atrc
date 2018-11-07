#include "ParamParser.h"

using namespace Atrc;

Spectrum ParamParser::ParseSpectrum(const ConfigNode &node)
{
	if(node.IsValue())
		return Spectrum(node.AsValue().Parse<float>());

	if(node.IsArray())
	{
		auto &arr = node.AsArray();

		if(!arr.Size())
			return Spectrum(0.0f);

		if(arr.Size() == 1)
			return Spectrum(arr[0].AsValue().Parse<float>());

		if(arr[0].AsValue() == "b")
		{
			if(arr.Size() == 2)
				return Spectrum(arr[1].AsValue().Parse<float>() / 255.0f);
			if(arr.Size() == 4)
				return 1 / 255.0f * Spectrum(arr[1].AsValue().Parse<float>(),
										     arr[2].AsValue().Parse<float>(),
										     arr[3].AsValue().Parse<float>());
		}
		else
		{
			if(arr.Size() == 3)
				return Spectrum(arr[0].AsValue().Parse<float>(),
								arr[1].AsValue().Parse<float>(),
								arr[2].AsValue().Parse<float>());
		}
	}

	throw ParamParsingError("Spectrum");
}
