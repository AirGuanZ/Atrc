#include <map>

#include <Atrc/Lib/Entity/GeometricDiffuseLight.h>
#include <Atrc/Lib/Entity/GeometricEntity.h>
#include <Atrc/Lib/Entity/GeometryGroupEntity.h>
#include <Atrc/Mgr/BuiltinCreator/EntityCreator.h>
#include <Atrc/Mgr/Parser.h>

namespace Atrc::Mgr
{

namespace
{
    MediumInterface CreateMediumInterface(const ConfigGroup &group, Context &context, [[maybe_unused]] Arena &arena)
    {
        ATRC_MGR_TRY
        {
            MediumInterface ret;
            if(auto in = group.Find("medium.in"))
                ret.in = context.Create<Medium>(*in);
            if(auto out = group.Find("medium.out"))
                ret.out = context.Create<Medium>(*out);
            return ret;
        }
        ATRC_MGR_CATCH_AND_RETHROW("In creating medium interface: " + group.ToString())
    }
}

void RegisterBuiltinEntityCreators(Context &context)
{
    static const GeometricDiffuseLightCreator geometricDiffuseLightCreator;
    static const GeometricEntityCreator geometricEntityCreator;
    static const GeometryGroupEntityCreator geometryGroupEntityCreator;
    context.AddCreator(&geometricDiffuseLightCreator);
    context.AddCreator(&geometricEntityCreator);
    context.AddCreator(&geometryGroupEntityCreator);
}

Entity* GeometricDiffuseLightCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        auto geometry = context.Create<Geometry>(group["geometry"]);
        auto radiance = Parser::ParseSpectrum(group["radiance"]);
        return arena.Create<GeometricDiffuseLight>(geometry, radiance);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating geometric diffuse light: " + group.ToString())
}

Entity *GeometricEntityCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        auto geometry = context.Create<Geometry>(group["geometry"]);
        auto material = context.Create<Material>(group["material"]);
        auto mediumInterface = CreateMediumInterface(group, context, arena);
        return arena.Create<GeometricEntity>(geometry, material, mediumInterface);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating geometric entity: " + group.ToString())
}

Entity *GeometryGroupEntityCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        auto name2Geometry = context.Create<Name2Geometry>(group["geometryGroup"]);
        auto &matAssignment = group["materialAssignment"].AsGroup();
        
        const ConfigGroup *medAssignment = nullptr;
        if(auto node = group.Find("mediumAssignment"))
            medAssignment = &node->AsGroup();

        auto geometry = arena.CreateArray<const Geometry*>(name2Geometry->size());
        auto material = arena.CreateArray<const Material*>(name2Geometry->size());
        auto medium   = arena.CreateArray<MediumInterface>(name2Geometry->size());

        size_t idx = 0;
        for(auto &pair : *name2Geometry)
        {
            geometry[idx] = pair.second;
            material[idx] = context.Create<Material>(matAssignment[pair.first]);

            if(medAssignment)
            {
                if(auto node = medAssignment->Find(pair.first))
                {
                    const Medium *in = nullptr, *out = nullptr;
                    if(auto inN = node->AsGroup().Find("in"))
                        in = context.Create<Medium>(*inN);
                    if(auto outN = node->AsGroup().Find("out"))
                        out = context.Create<Medium>(*outN);
                    medium[idx] = MediumInterface{ in, out };
                }
            }

            ++idx;
        }

        auto transform = Parser::ParseTransform(group["transform"]);

        return arena.Create<GeometryGroupEntity>(
            geometry, material, medium, int(name2Geometry->size()),
            transform);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating geometry group entity: " + group.ToString())
}

} // namespace Atrc::Mgr
