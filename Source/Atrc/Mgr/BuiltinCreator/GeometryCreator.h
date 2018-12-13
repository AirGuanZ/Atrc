#pragma once

#include <Atrc/Mgr/Context.h>

namespace Atrc::Mgr
{

/*
    type = Sphere

    radius    = Real
    transform = Transform
*/
class SphereCreator : public Creator<Geometry>
{
public:

    Str8 GetTypeName() const override { return "Sphere"; }

    const Geometry *Create(const ConfigGroup &group, Context &context, Arena &arena) override;
};

} // namespace Atrc::Mgr
