#pragma once

#include <Atrc/Mgr/Context.h>

namespace Atrc::Mgr
{

void RegisterBuiltinEntityCreators(Context &context);

/*
    type     = Geometric
    
    geometry = GeometryDefinition
    material = MaterialDefinition
*/
class GeometricEntityCreator : public Creator<Entity>
{
public:

    Str8 GetTypeName() const override { return "Geometric"; }

    Entity *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

} // namespace Atrc::Mgr
