#include <Utils/Model.h>

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
    Name2Geometry RecreateName2Geometry(const Str8 &filename, const Str8 &cacheFilename, Arena &arena)
    {
        Name2Geometry ret;

        // 加载原obj文件

        AGZ::Model::WavefrontObj<Real> obj;
        if(!AGZ::Model::WavefrontObjFile<Real>::LoadFromObjFile(filename, &obj))
            throw MgrErr("Failed to load obj file from " + filename);
        auto meshGroup = obj.ToGeometryMeshGroup();
        obj.Clear();

        // 准备目标为cache file的serializer

        AGZ::FileSys::File::CreateDirectoryRecursively(AGZ::FileSys::Path8(cacheFilename).ToDirectory().ToStr());

        std::ofstream fout(cacheFilename.ToPlatformString(), std::ofstream::trunc | std::ofstream::binary);
        if(!fout)
            throw MgrErr("Failed to create new cache file: " + cacheFilename);

        auto oriFileTime = AGZ::FileSys::File::GetLastWriteTime(filename);
        if(!oriFileTime)
            throw MgrErr("Failed to load last write time of " + filename);

        AGZ::BinaryOStreamSerializer serializer(fout);

        if(!serializer.Serialize(*oriFileTime))
            throw MgrErr("Failed to serialize filetime into cache file: " + cacheFilename);

        if(!serializer.Serialize(uint32_t(meshGroup.submeshes.size())))
            throw MgrErr("Failed to serialize submesh count into cache file: " + cacheFilename);

        // 按字典序逐个处理submesh

        for(auto &pair : meshGroup.submeshes)
        {
            auto &mesh = pair.second;
            auto vertices = mesh.vertices | AGZ::Map([](const auto &v)
            {
                TriangleBVHCore::Vertex ret;
                ret.pos = v.pos;
                ret.nor = v.nor.Normalize();
                ret.uv = v.tex.xy();
                return ret;
            }) | AGZ::Collect<std::vector<TriangleBVHCore::Vertex>>();

            AGZ_ASSERT(vertices.size() % 3 == 0);

            auto triBVH = arena.Create<TriangleBVHCore>(vertices.data(), uint32_t(vertices.size() / 3));

            if(!serializer.Serialize(pair.first))
                throw MgrErr("Failed to serialize mesh name into cache file: " + pair.first);

            if(!serializer.Serialize(*triBVH))
                throw MgrErr("Failed to serialize triangle BVH data infor cache file for submesh: " + pair.first);
            
            ret[pair.first] = arena.Create<TriangleBVH>(Transform(), *triBVH);
        }

        return ret;
    }

    Name2Geometry LoadName2Geometry(const Str8 &filename, Arena &arena)
    {
        auto cacheFilename = GetCacheFilename(filename);
        std::ifstream fin(cacheFilename.ToPlatformString(), std::ifstream::binary | std::ifstream::in);
        if(!fin)
            return RecreateName2Geometry(filename, cacheFilename, arena);
        
        auto oriFileTime = AGZ::FileSys::File::GetLastWriteTime(filename);
        if(!oriFileTime)
            throw MgrErr("Failed to load last write time of " + filename);

        AGZ::BinaryIStreamDeserializer deserializer(fin);
        auto cacheTime = deserializer.DeserializeFromScratch<AGZ::FileSys::FileTime>();
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
            auto name = deserializer.DeserializeFromScratch<Str8>();
            auto core = deserializer.DeserializeFromScratch<TriangleBVHCore>();

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
        auto filename = group["filename"].AsValue();

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
