#include <Atrc/Lib/Entity/GeometricDiffuseLight.h>
#include <Atrc/Lib/Entity/GeometricEntity.h>
#include <Atrc/Mgr/BuiltinCreator/EntityCreator.h>
#include <Atrc/Mgr/Parser.h>

namespace Atrc::Mgr
{

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
        return arena.Create<GeometricEntity>(geometry, material);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating geometric entity: " + group.ToString())
}

} // namespace Atrc::Mgr
