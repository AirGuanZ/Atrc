#include <Atrc/Utility/ParamParser.h>

AGZ_NS_BEG(Atrc)

float ParamParser::ParseFloat(const StrView8 &s)
{
    try
    {
        return s.Parse<float>();
    }
    catch(...)
    {
        throw ParamParser_SyntaxError("Syntax error in float number");
    }
}

Real ParamParser::ParseReal(const StrView8 &s)
{
    try
    {
        return s.Parse<Real>();
    }
    catch(...)
    {
        throw ParamParser_SyntaxError("Syntax error in real number");
    }
}

Spectrum ParamParser::ParseRGB3f(const StrView8 &s)
{
    static thread_local AGZ::Regex8 reg(R"___(\(\s*&@{!,}+&\s*,\s*&@{!,}+&\s*,\s*&@{!,}+&\s*\))___");
    auto m = reg.Match(s);
    if(!m)
        throw ParamParser_SyntaxError("Syntax error in RGB3f");

    Spectrum ret;

    try
    {
        ret.r = m(0, 1).Parse<float>();
        ret.g = m(2, 3).Parse<float>();
        ret.b = m(4, 5).Parse<float>();
    }
    catch(...)
    {
        throw ParamParser_SyntaxError("Syntax error in RGB3f");
    }

    return ret;
}

Spectrum ParamParser::ParseRGB3b(const StrView8 &s)
{
    static thread_local AGZ::Regex8 reg(R"___(\(\s*&\d+&\s*,\s*&\d+&\s*,\s*&\d+&\s*\)b)___");
    auto m = reg.Match(s);
    if(!m)
        throw ParamParser_SyntaxError("Syntax error in RGB3f");

    Spectrum ret;

    try
    {
        ret.r = m(0, 1).Parse<int>() / 255.0f;
        ret.g = m(2, 3).Parse<int>() / 255.0f;
        ret.b = m(4, 5).Parse<int>() / 255.0f;
    }
    catch(...)
    {
        throw ParamParser_SyntaxError("Syntax error in RGB3b");
    }

    return Clamp(ret, 0.0f, 1.0f);
}

Spectrum ParamParser::ParseRGB(const StrView8 &s)
{
    try
    {
        return ParseRGB3f(s);
    }
    catch(...)
    {
        // do nothing
    }

    try
    {
        return ParseRGB3b(s);
    }
    catch(...)
    {
        throw ParamParser_SyntaxError("Syntax error in RGB3");
    }
}

Vec3 ParamParser::ParseVec3(const StrView8 &s)
{
    static thread_local AGZ::Regex8 reg(R"___(\(\s*&@{!,}+&\s*,\s*&@{!,}+&\s*,\s*&@{!,}+&\s*\))___");
    auto m = reg.Match(s);
    if(!m)
        throw ParamParser_SyntaxError("Syntax error in Vec3");

    Vec3 ret;

    try
    {
        ret.x = m(0, 1).Parse<Real>();
        ret.y = m(2, 3).Parse<Real>();
        ret.z = m(4, 5).Parse<Real>();
    }
    catch(...)
    {
        throw ParamParser_SyntaxError("Syntax error in Vec3");
    }

    return ret;
}

AGZ_NS_END(Atrc)
