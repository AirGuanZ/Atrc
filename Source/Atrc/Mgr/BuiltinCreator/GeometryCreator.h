#pragma once

#include <unordered_map>

#include <Atrc/Mgr/Context.h>

namespace Atrc
{
    class TriangleBVHCore;
}


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

/*
    type = TriangleBVH

    filename  = Filename
    transform = Transform
*/
class TriangleBVHCreator : public Creator<Geometry>
{
    mutable std::unordered_map<Str8, const TriangleBVHCore*> path2Core_;

public:

    Str8 GetTypeName() const override { return "TriangleBVH"; }

    Geometry *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

} // namespace Atrc::Mgr
