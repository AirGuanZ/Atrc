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
*/
class IdealDiffuseCreator : public Creator<Material>
{
public:

    Str8 GetTypeName() const override { return "IdealDiffuse"; }

    Material *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

} // namespace Atrc::Mgr
