#pragma once

#include <Atrc/Mgr/Context.h>

namespace Atrc::Mgr
{

void RegisterBuiltinGeometryCreators(Context &context);

/*
    type = Sphere

    radius    = Real
    transform = Transform
*/
class SphereCreator : public Creator<Geometry>
{
public:

    Str8 GetTypeName() const override { return "Sphere"; }

    Geometry *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

} // namespace Atrc::Mgr
