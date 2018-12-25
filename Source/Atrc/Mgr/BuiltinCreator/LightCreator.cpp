#include <Atrc/Lib/Light/CubeEnvironmentLight.h>
#include <Atrc/Lib/Light/EnvironmentLight.h>
#include <Atrc/Lib/Light/SkyLight.h>
#include <Atrc/Mgr/BuiltinCreator/LightCreator.h>
#include <Atrc/Mgr/Parser.h>

namespace Atrc::Mgr
{

void RegisterBuiltinLightCreators(Context &context)
{
    static const CubeEnvironmentLightCreator cubeEnvironmentLight;
    static const EnvironmentLightCreator environmentLightCreator;
    static const SkyLightCreator skyLightCreator;
    context.AddCreator(&cubeEnvironmentLight);
    context.AddCreator(&environmentLightCreator);
    context.AddCreator(&skyLightCreator);
}

Light *CubeEnvironmentLightCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        auto posX = context.Create<Texture>(group["posX"]);
        auto posY = context.Create<Texture>(group["posY"]);
        auto posZ = context.Create<Texture>(group["posZ"]);
        auto negX = context.Create<Texture>(group["negX"]);
        auto negY = context.Create<Texture>(group["negY"]);
        auto negZ = context.Create<Texture>(group["negZ"]);
        const Texture *envTex[] =
        {
            posX, posY, posZ, negX, negY, negZ
        };

        Transform transform;
        if(auto tNode = group.Find("transform"))
            transform = Parser::ParseTransform(*tNode);

        return arena.Create<CubeEnvironmentLight>(envTex, transform);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating cube environment light: " + group.ToString())
}

Light *EnvironmentLightCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        auto tex = context.Create<Texture>(group["tex"]);

        Transform transform;
        if(auto tNode = group.Find("transform"))
            transform = Parser::ParseTransform(*tNode);

        return arena.Create<EnvironmentLight>(tex, transform);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating environment light: " + group.ToString())
}

Light *SkyLightCreator::Create(const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        auto top    = Parser::ParseSpectrum(group["top"]);
        auto bottom = Parser::ParseSpectrum(group["bottom"]);
        
        Transform transform;
        if(auto tNode = group.Find("transform"))
            transform = Parser::ParseTransform(*tNode);

        return arena.Create<SkyLight>(top, bottom, transform);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating sky light: " + group.ToString())
}

} // namespace Atrc::Mgr
