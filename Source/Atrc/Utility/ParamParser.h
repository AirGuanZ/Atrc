#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

DEFINE_ATRC_EXCEPTION(ParamParser_SyntaxError);

class ParamParser
{
public:

    static float ParseFloat(const StrView8 &s);

    static Real ParseReal(const StrView8 &s);

    // "(0.6, 0.8, 0.1)" => Spectrum(0.6f, 0.8f, 0.1f)
    static Spectrum ParseRGB3f(const StrView8 &s);

    // "(168, 255, 12)b" => Spectrum(168.0f, 255.0f, 12.0f) / 255.0f
    static Spectrum ParseRGB3b(const StrView8 &s);

    // ParseRGB3f | ParseRGB3b
    static Spectrum ParseRGB(const StrView8 &s);

    // "(12.04, 1.5, 0.74)" => Vec3(12.04, 1.5, 0.74)
    static Vec3 ParseVec3(const StrView8 &s);
};

AGZ_NS_END(Atrc)
