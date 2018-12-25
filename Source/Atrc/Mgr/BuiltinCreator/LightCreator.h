#pragma once

#include <Atrc/Mgr/Context.h>

namespace Atrc::Mgr
{

void RegisterBuiltinLightCreators(Context &context);

// type = CubeEnvironment
// posX = Texture
// posY = Texture
// posZ = Texture
// negX = Texture
// negY = Texture
// negZ = Texture
class CubeEnvironmentLightCreator : public Creator<Light>
{
public:

    Str8 GetTypeName() const override { return "CubeEnvironment"; }

    Light *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

/*
    type = Environment

    tex = Texture
*/
class EnvironmentLightCreator : public Creator<Light>
{
public:

    Str8 GetTypeName() const override { return "Environment"; }

    Light *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

/*
    type = Sky

    top    = Spectrum
    bottom = Spectrum
*/
class SkyLightCreator : public Creator<Light>
{
public:

    Str8 GetTypeName() const override { return "Sky"; }

    Light *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

} // namespace Atrc::Mgr
