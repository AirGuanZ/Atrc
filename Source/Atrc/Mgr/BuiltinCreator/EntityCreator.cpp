#include <Atrc/Core/Entity/GeometricDiffuseLight.h>
#include <Atrc/Core/Entity/GeometricEntity.h>
#include <Atrc/Core/Entity/TwoSideEntity.h>
#include <Atrc/Mgr/BuiltinCreator/EntityCreator.h>
#include <Atrc/Mgr/Parser.h>

namespace Atrc::Mgr
{

namespace
{
    MediumInterface CreateMediumInterface(const ConfigGroup &group, Context &context, [[maybe_unused]] Arena &arena)
    {
        AGZ_HIERARCHY_TRY

        MediumInterface ret;
        if(auto in = group.Find("medium.in"))
            ret.in = context.Create<Medium>(*in);
        if(auto out = group.Find("medium.out"))
            ret.out = context.Create<Medium>(*out);
        return ret;

        AGZ_HIERARCHY_WRAP("In creating medium interface: " + group.ToString())
    }
}

void RegisterBuiltinEntityCreators(Context &context)
{
    static const GeometricDiffuseLightCreator geometricDiffuseLightCreator;
    static const GeometricEntityCreator geometricEntityCreator;
    static const TwoSideEntityCreator iTwoSideEntityCreator;
    context.AddCreator(&geometricDiffuseLightCreator);
    context.AddCreator(&geometricEntityCreator);
    context.AddCreator(&iTwoSideEntityCreator);
}

Entity* GeometricDiffuseLightCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY

    auto geometry = context.Create<Geometry>(group["geometry"]);
    auto radiance = Parser::ParseSpectrum(group["radiance"]);
    return arena.Create<GeometricDiffuseLight>(geometry, radiance);

    AGZ_HIERARCHY_WRAP("In creating geometric diffuse light: " + group.ToString())
}

Entity *GeometricEntityCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY

    auto geometry = context.Create<Geometry>(group["geometry"]);
    auto material = context.Create<Material>(group["material"]);
    auto mediumInterface = CreateMediumInterface(group, context, arena);
    return arena.Create<GeometricEntity>(geometry, material, mediumInterface);

    AGZ_HIERARCHY_WRAP("In creating geometric entity: " + group.ToString())
}

Entity *TwoSideEntityCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY

    auto internalEntity = context.Create<Entity>(group["internal"]);
    return arena.Create<TwoSideEntity>(internalEntity);

    AGZ_HIERARCHY_WRAP("In creating two side geometry entity: " + group.ToString())
}

} // namespace Atrc::Mgr
