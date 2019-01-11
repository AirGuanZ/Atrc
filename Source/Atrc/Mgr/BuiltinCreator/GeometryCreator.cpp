#include <AGZUtils/Utils/Mesh.h>
#include <AGZUtils/Utils/Range.h>

#include <vector>

#include <Atrc/Lib/Geometry/Sphere.h>
#include <Atrc/Lib/Geometry/Triangle.h>
#include <Atrc/Lib/Geometry/TriangleBVH.h>
#include <Atrc/Mgr/BuiltinCreator/GeometryCreator.h>
#include <Atrc/Mgr/Parser.h>

namespace Atrc::Mgr
{

void RegisterBuiltinGeometryCreators(Context &context)
{
    static const SphereCreator sphereCreator;
    static const TriangleCreator triangleCreator;
    context.AddCreator(&sphereCreator);
    context.AddCreator(&triangleCreator);
    context.AddCreator(context.CreateWithInteriorArena<TriangleBVHCreator>());
}

Geometry *SphereCreator::Create(const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        Real radius    = group["radius"].Parse<Real>();
        auto transform = Parser::ParseTransform(group["transform"]);
        return arena.Create<Sphere>(transform, radius);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating sphere: " + group.ToString())
}

Geometry *TriangleCreator::Create(const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        Vec3 A = Parser::ParseVec3(group["A"]);
        Vec3 B = Parser::ParseVec3(group["B"]);
        Vec3 C = Parser::ParseVec3(group["C"]);
        Vec2 tA = Parser::ParseVec2(group["tA"]);
        Vec2 tB = Parser::ParseVec2(group["tB"]);
        Vec2 tC = Parser::ParseVec2(group["tC"]);
        auto transform = Parser::ParseTransform(group["transform"]);
        return arena.Create<Triangle>(transform, A, B, C, tA, tB, tC);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating triangle: " + group.ToString())
}

namespace
{
    TriangleBVHCore *RecreateTriangleMesh(const Str8 &filename, const Str8 &cacheFilename, Arena &arena)
    {
        /*AGZ::Model::WavefrontObj<Real> obj;
        if(!AGZ::Model::WavefrontObjFile<Real>::LoadFromObjFile(filename, &obj))
            throw MgrErr("Failed to load obj file from " + filename);*/
        AGZ::Mesh::WavefrontObj<Real> obj;
        if(!obj.LoadFromFile(filename))
            throw MgrErr("Failed to load obj file from " + filename);
        auto mesh = obj.ToGeometryMeshGroup().MergeAllSubmeshes();
        obj.Clear();

        auto vertices = mesh.vertices | AGZ::Map([](const auto &v)
        {
            TriangleBVHCore::Vertex ret;
            ret.pos = v.pos;
            ret.nor = v.nor.Normalize();
            ret.uv = v.tex.xy();
            return ret;
        }) | AGZ::Collect<std::vector<TriangleBVHCore::Vertex>>();

        AGZ_ASSERT(vertices.size() % 3 == 0);

        auto ret = arena.Create<TriangleBVHCore>(vertices.data(), uint32_t(vertices.size() / 3));

        AGZ::FileSys::File::CreateDirectoryRecursively(AGZ::FileSys::Path8(cacheFilename).ToDirectory().ToStr());

        std::ofstream fout(cacheFilename.ToPlatformString(), std::ofstream::trunc | std::ofstream::binary);
        if(!fout)
            throw MgrErr("Failed to create new triangle mesh cache file: " + cacheFilename);

        auto oriFileTime = AGZ::FileSys::File::GetLastWriteTime(filename);
        if(!oriFileTime)
            throw MgrErr("Failed to load last write time of " + filename);

        AGZ::BinaryOStreamSerializer serializer(fout);
        if(!serializer.Serialize(*oriFileTime) || !serializer.Serialize(*ret))
        {
            AGZ::FileSys::File::DeleteRegularFile(cacheFilename);
            throw MgrErr("Failed to serialize into triangle mesh cache file: " + cacheFilename);
        }

        return ret;
    }

    TriangleBVHCore *LoadTriangleMesh(const Str8 &filename, Arena &arena)
    {
        auto cacheFilename = GetCacheFilename(filename);
        std::ifstream fin(cacheFilename.ToPlatformString(), std::ifstream::binary | std::ifstream::in);
        if(!fin)
            return RecreateTriangleMesh(filename, cacheFilename, arena);

        auto oriFileTime = AGZ::FileSys::File::GetLastWriteTime(filename);
        if(!oriFileTime)
            throw MgrErr("Failed to load last write time of " + filename);

        AGZ::BinaryIStreamDeserializer deserializer(fin);
        auto cacheTime = deserializer.DeserializeFromScratch<AGZ::FileSys::FileTime>();

        if(!cacheTime || *cacheTime != *oriFileTime)
        {
            fin.close();
            return RecreateTriangleMesh(filename, cacheFilename, arena);
        }

        auto mesh = deserializer.DeserializeFromScratch<TriangleBVHCore>();
        if(!mesh)
        {
            fin.close();
            return RecreateTriangleMesh(filename, cacheFilename, arena);
        }

        return arena.Create<TriangleBVHCore>(std::move(*mesh));
    }
}

Geometry *TriangleBVHCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        auto transform = Parser::ParseTransform(group["transform"]);

        const Str8 filename = context.GetPathInWorkspace(group["filename"].AsValue());
        const TriangleBVHCore *core;

        auto it = path2Core_.find(filename);
        if(it != path2Core_.end())
        {
            core = it->second;
        }
        else
        {
            core = LoadTriangleMesh(filename, arena);
            path2Core_[filename] = core;
        }

        return arena.Create<TriangleBVH>(transform, *core);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating triangle bvh: " + group.ToString())
}

} // namespace Atrc::Mgr
