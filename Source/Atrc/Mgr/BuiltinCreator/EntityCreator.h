#pragma once

#include <Atrc/Mgr/Context.h>

namespace Atrc::Mgr
{

/*
    type     = Geometric
    
    geometry = GeometryDefinition
    material = MaterialDefinition
*/
class GeometricEntityCreator : public Creator<Entity>
{
public:

    Str8 GetTypeName() const override { return "Geometric"; }

    const Entity *Create(const ConfigGroup &group, Context &context, Arena &arena) override;
};

} // namespace Atrc::Mgr
