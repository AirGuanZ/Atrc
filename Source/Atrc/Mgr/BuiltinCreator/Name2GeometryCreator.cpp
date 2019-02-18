#include <filesystem>

#include <AGZUtils/Utils/FileSys.h>
#include <AGZUtils/Utils/Mesh.h>

#include <Atrc/Lib/Geometry/TriangleBVH.h>
#include <Atrc/Mgr/BuiltinCreator/Name2GeometryCreator.h>

namespace Atrc::Mgr
{

void RegisterBuiltinName2GeometryCreators(Context &context)
{
    context.AddCreator(context.CreateWithInteriorArena<WavefrontOBJName2GeometryCreator>());
}

namespace
{
    Name2Geometry RecreateName2Geometry(std::string_view filename, std::string_view cacheFilename, Arena &arena)
    {
        Name2Geometry ret;

        // 加载原obj文件

        AGZ::Mesh::WavefrontObj<Real> obj;
        if(!obj.LoadFromFile(filename))
            throw MgrErr("Failed to load obj file from " + std::string(filename));
        auto meshGroup = obj.ToGeometryMeshGroup();
        obj.Clear();

        // 准备目标为cache file的serializer

        AGZ::FileSys::File::CreateDirectoryRecursively(std::filesystem::path(WIDEN(cacheFilename)).parent_path().string());

        std::ofstream fout(WIDEN(cacheFilename), std::ofstream::trunc | std::ofstream::binary);
        if(!fout)
            throw MgrErr("Failed to create new cache file: " + std::string(cacheFilename));

        auto oriFileTime = AGZ::FileSys::File::GetLastWriteTime(filename);
        if(!oriFileTime)
            throw MgrErr("Failed to load last write time of " + std::string(filename));

        AGZ::BinaryOStreamSerializer serializer(fout);

        if(!serializer.Serialize(*oriFileTime))
        {
            AGZ::FileSys::File::DeleteRegularFile(cacheFilename);
            throw MgrErr("Failed to serialize filetime into cache file: " + std::string(cacheFilename));
        }

        if(!serializer.Serialize(uint32_t(meshGroup.submeshes.size())))
        {
            AGZ::FileSys::File::DeleteRegularFile(cacheFilename);
            throw MgrErr("Failed to serialize submesh count into cache file: " + std::string(cacheFilename));
        }

        // 按字典序逐个处理submesh

        for(auto &pair : meshGroup.submeshes)
        {
            auto &mesh = pair.second;
            auto vertices = mesh.vertices | AGZ::Map([](const auto &v)
            {
                TriangleBVHCore::Vertex ret;
                ret.pos = v.pos;
                ret.nor = v.nor.Normalize();
                ret.uv  = v.tex.xy();
                return ret;
            }) | AGZ::Collect<std::vector<TriangleBVHCore::Vertex>>();

            AGZ_ASSERT(vertices.size() % 3 == 0);

            auto triBVH = arena.Create<TriangleBVHCore>(vertices.data(), uint32_t(vertices.size() / 3));

            if(!serializer.Serialize(pair.first))
            {
                AGZ::FileSys::File::DeleteRegularFile(cacheFilename);
                throw MgrErr("Failed to serialize mesh name into cache file: " + pair.first);
            }

            if(!serializer.Serialize(*triBVH))
            {
                AGZ::FileSys::File::DeleteRegularFile(cacheFilename);
                throw MgrErr("Failed to serialize triangle BVH data infor cache file for submesh: " + pair.first);
            }
            
            ret[pair.first] = arena.Create<TriangleBVH>(Transform(), *triBVH);
        }

        return ret;
    }

    Name2Geometry LoadName2Geometry(std::string_view filename, Arena &arena)
    {
        auto cacheFilename = GetCacheFilename(filename) + ".geometry-group";
        std::ifstream fin(WIDEN(cacheFilename), std::ifstream::binary | std::ifstream::in);
        if(!fin)
            return RecreateName2Geometry(filename, cacheFilename, arena);
        
        auto oriFileTime = AGZ::FileSys::File::GetLastWriteTime(filename);
        if(!oriFileTime)
            throw MgrErr("Failed to load last write time of " + std::string(filename));

        AGZ::BinaryIStreamDeserializer deserializer(fin);
        auto cacheTime = deserializer.Deserialize<AGZ::FileSys::FileTime>();
        if(!cacheTime || *cacheTime != *oriFileTime)
        {
            fin.close();
            return RecreateName2Geometry(filename, cacheFilename, arena);
        }

        uint32_t submeshCount;
        if(!deserializer.Deserialize(submeshCount))
            throw MgrErr("Failed to deserialize submesh count from " + cacheFilename);
        
        Name2Geometry ret;
        for(uint32_t i = 0; i < submeshCount; ++i)
        {
            auto name = deserializer.Deserialize<std::string>();
            auto core = deserializer.Deserialize<TriangleBVHCore>();

            if(!name || !core)
                throw MgrErr("Failed to deserialize submesh from " + cacheFilename);
            
            auto pCore = arena.Create<TriangleBVHCore>(std::move(*core));
            ret[*name] = arena.Create<TriangleBVH>(Transform(), *pCore);
        }

        return ret;
    }
}

Name2Geometry *WavefrontOBJName2GeometryCreator::Create(
    const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        auto filename = context.GetPathInWorkspace(group["filename"].AsValue());

        auto it = path2rt_.find(filename);
        if(it != path2rt_.end())
            return it->second;
        
        auto ret = arena.Create<Name2Geometry>(LoadName2Geometry(filename, arena));
        path2rt_[filename] = ret;

        return ret;
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating WavefrontOBJ Name2Geometry mapping: " + group.ToString())
}

} // namespace Atrc::Mgr
