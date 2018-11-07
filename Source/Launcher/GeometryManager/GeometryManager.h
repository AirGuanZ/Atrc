#pragma once

#include "../Common.h"

using GeometryCreator = ObjectCreator<Atrc::Geometry>;
using GeometryManager = ObjectManager<Atrc::Geometry>;

// radius = Real
// transform = Transform
class SphereCreator : public GeometryCreator, public AGZ::Singleton<SphereCreator>
{
public:

	Str8 GetName() const override { return "Sphere"; }

	Atrc::Geometry *Create(const ConfigGroup &params, ObjArena<> &arena) const override;
};

// sidelen = Real
// transform = Transform
class CubeCreator : public GeometryCreator, public AGZ::Singleton<SphereCreator>
{
public:

	Str8 GetName() const override { return "Cube"; }

	Atrc::Geometry *Create(const ConfigGroup &params, ObjArena<> &arena) const override;
};

// path = ...
// transform = Transform
class TriangleBVHCreator : public GeometryCreator, public AGZ::Singleton<SphereCreator>
{
	mutable std::unordered_map<Str8, const Atrc::TriangleBVHCore*> path2Core_;

public:

	Str8 GetName() const override { return "TriangleBVH"; }

	Atrc::Geometry *Create(const ConfigGroup &params, ObjArena<> &arena) const override;
};
