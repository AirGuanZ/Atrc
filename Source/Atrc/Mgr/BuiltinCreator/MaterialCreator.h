#pragma once

#include <Atrc/Mgr/Context.h>

namespace Atrc::Mgr
{

/*
    type = IdealBlack
*/
class IdealBlackCreator : public Creator<Material>
{
public:

    Str8 GetTypeName() const override { return "IdealBlack"; }

    const Material *Create(const ConfigGroup &group, Context &context, Arena &arena) override;
};

/*
    type = IdealDiffuse

    albedo = Texture
*/
class IdealDiffuseCreator : public Creator<Material>
{
public:

    Str8 GetTypeName() const override { return "IdealDiffuse"; }

    const Material *Create(const ConfigGroup &group, Context &context, Arena &arena) override;
};

} // namespace Atrc::Mgr
