#include <map>

#include <Atrc/Lib/Entity/GeometricDiffuseLight.h>
#include <Atrc/Lib/Entity/GeometricEntity.h>
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
    context.AddCreator(&geometricDiffuseLightCreator);
    context.AddCreator(&geometricEntityCreator);
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

} // namespace Atrc::Mgr
