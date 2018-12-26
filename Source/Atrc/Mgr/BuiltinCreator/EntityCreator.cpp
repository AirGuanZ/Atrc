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

        auto geometry = arena.Create<std::vector<const Geometry*>>(name2Geometry->size());
        auto material = arena.Create<std::vector<const Material*>>(name2Geometry->size());
        auto medium   = arena.Create<std::vector<const MediumInterface*>>(name2Geometry->size());

        size_t idx = 0;
        for(auto &pair : *name2Geometry)
        {
            geometry->at(idx) = pair.second;
            material->at(idx) = context.Create<Material>(matAssignment[pair.first]);
            
            static MediumInterface EMPTY_MEDIUM_INTERFACE;
            medium->at(idx) = &EMPTY_MEDIUM_INTERFACE;

            if(medAssignment)
            {
                if(auto node = medAssignment->Find(pair.first))
                {
                    auto medInterface = arena.Create<MediumInterface>();
                    if(auto in = node->AsGroup().Find("in"))
                        medInterface->in = context.Create<Medium>(*in);
                    if(auto out = node->AsGroup().Find("out"))
                        medInterface->out = context.Create<Medium>(*out);
                    medium->at(idx) = medInterface;
                }
            }

            ++idx;
        }

        auto transform = Parser::ParseTransform(group["transform"]);

        return arena.Create<GeometryGroupEntity>(
            geometry->data(), material->data(), medium->data(), int(name2Geometry->size()),
            transform);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating geometry group entity: " + group.ToString())
}

} // namespace Atrc::Mgr
