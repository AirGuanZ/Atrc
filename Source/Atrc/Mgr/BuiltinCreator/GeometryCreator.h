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
    type = Triangle

    A = Vec3
    B = Vec3
    C = Vec3
    tA = Vec2
    tB = Vec2
    tC = Vec2
    transform = Transform
*/
class TriangleCreator : public Creator<Geometry>
{
public:

    Str8 GetTypeName() const override { return "Triangle"; }

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
