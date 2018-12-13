#pragma once

#include <Atrc/Mgr/Context.h>

namespace Atrc::Mgr
{

/*
    type = Sky

    top    = Spectrum
    bottom = Spectrum
*/
class SkyLightCreator : public Creator<Light>
{
public:

    Str8 GetTypeName() const override { return "Sky"; }

    const Light *Create(const ConfigGroup &group, Context &context, Arena &arena) override;
};

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

    const Light *Create(const ConfigGroup &group, Context &context, Arena &arena) override;
};

} // namespace Atrc::Mgr
