#include <Utils/Model.h>
#include <Utils/Range.h>

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

Geometry *TriangleCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
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

Geometry *TriangleBVHCreator::Create(const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        auto transform = Parser::ParseTransform(group["transform"]);

        const Str8 &filename = group["filename"].AsValue();
        const TriangleBVHCore *core;

        auto it = path2Core_.find(filename);
        if(it != path2Core_.end())
        {
            core = it->second;
        }
        else
        {
            AGZ::Model::WavefrontObj<Real> obj;
            if(!AGZ::Model::WavefrontObjFile<Real>::LoadFromObjFile(filename, &obj))
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

            core = arena.Create<TriangleBVHCore>(
                vertices.data(), uint32_t(vertices.size() / 3));
            path2Core_[filename] = core;
        }

        return arena.Create<TriangleBVH>(transform, *core);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating triangle bvh: " + group.ToString())
}

} // namespace Atrc::Mgr
