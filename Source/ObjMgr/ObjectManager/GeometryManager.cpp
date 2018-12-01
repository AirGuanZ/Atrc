#include "../ParamParser.h"
#include "GeometryManager.h"

AGZ_NS_BEG(ObjMgr)

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
        auto cacheFilename = AGZ::FileSys::BinaryFileCache::AutoCacheName(path);
        auto newCore = AGZ::FileSys::BinaryFileCache::Cache(
            cacheFilename,
            [&](AGZ::BinarySerializer &s) // cache builder
            {
                s.Serialize(*AGZ::FileSys::File::GetLastWriteTime(path));

                AGZ::Model::WavefrontObj<Atrc::Real> objs;
                if(!AGZ::Model::WavefrontObjFile<Atrc::Real>::LoadFromObjFile(path, &objs))
                {
                    throw SceneInitializationException(
                        "Failed to load obj model from " + path);
                }

                auto ret = arena.Create<Atrc::TriangleBVHCore>(
                        objs.ToGeometryMeshGroup().MergeAllSubmeshes());
                s.Serialize(*ret);

                return ret;
            },
            [&](AGZ::BinaryDeserializer &d) // cache validator
            {
                AGZ::FileSys::FileTime timeInCache;
                d.Deserialize(timeInCache);
                auto pathWriteTime = AGZ::FileSys::File::GetLastWriteTime(path);
                if(!pathWriteTime)
                    return false;
                return d.Ok() && timeInCache == *pathWriteTime;
            },
            [&](AGZ::BinaryDeserializer &d) // cache loader
            {
                auto ret = arena.Create<Atrc::TriangleBVHCore>();
                if(!d.Deserialize(*ret))
                {
                    throw SceneInitializationException(
                        "Failed to deserialize triangle BVH from: " + cacheFilename);
                }
                return ret;
            });

        path2Core_[path] = newCore;
        core = newCore;
    }
    else
        core = it->second;

    return arena.Create<Atrc::TriangleBVH>(transform, core);
}

AGZ_NS_END(ObjMgr)
