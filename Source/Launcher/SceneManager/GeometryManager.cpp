#include "../ParamParser/ParamParser.h"
#include "GeometryManager.h"

Atrc::Geometry *SphereCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
	auto radius = params["radius"].AsValue().Parse<Atrc::Real>();
	auto transform = ParamParser::ParseTransform(params["transform"]);

	return arena.Create<Atrc::Sphere>(transform, radius);
}

Atrc::Geometry *CubeCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
	auto sidelen = params["sidelen"].AsValue().Parse<Atrc::Real>();
	auto transform = ParamParser::ParseTransform(params["transform"]);

	return arena.Create<Atrc::Cube>(transform, sidelen);
}

Atrc::Geometry *TriangleBVHCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
	auto path = params["path"].AsValue();
	auto transform = ParamParser::ParseTransform(params["transform"]);

	const Atrc::TriangleBVHCore *core;

	auto it = path2Core_.find(path);
	if(it == path2Core_.end())
	{
		AGZ::Model::WavefrontObj objs;
		if(!AGZ::Model::WavefrontObjFile::LoadFromObjFile(
			path.ToStdWString(), &objs))
		{
			throw SceneInitializationException(
                "Failed to load obj model from " + path);
		}

		auto newCore = arena.Create<Atrc::TriangleBVHCore>(
			objs.ToGeometryMeshGroup().MergeAllSubmeshes());
		path2Core_[path] = newCore;
		core = newCore;
	}
	else
		core = it->second;

	return arena.Create<Atrc::TriangleBVH>(transform, core);
}
