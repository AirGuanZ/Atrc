#pragma once

#include <Atrc/Mgr/Context.h>

namespace Atrc::Mgr
{

void RegisterBuiltinMaterialCreators(Context &context);

/*
    type = IdealBlack
*/
class IdealBlackCreator : public Creator<Material>
{
public:

    Str8 GetTypeName() const override { return "IdealBlack"; }

    Material *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

/*
    type = IdealDiffuse

    albedo = Texture

    normalMapper = NormalMapper | null
*/
class IdealDiffuseCreator : public Creator<Material>
{
public:

    Str8 GetTypeName() const override { return "IdealDiffuse"; }

    Material *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

/*
    type = TSMetal

    etaI      = Spectrum
    etaT      = Spectrum
    k         = Spectrum
    rc        = Texture
    roughness = Texture

    normalMapper = NormalMapper | null
*/
class TSMetalCreator : public Creator<Material>
{
public:

    Str8 GetTypeName() const override { return "TSMetal"; }

    Material *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

/*
    type = ONMatte

    albedo = Texture
    sigma  = Texture

    normalMapper = NormalMapper | null
*/
class ONMatteCreator : public Creator<Material>
{
public:

    Str8 GetTypeName() const override { return "ONMatte"; }

    Material *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

} // namespace Atrc::Mgr
