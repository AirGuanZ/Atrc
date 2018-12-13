#include <Atrc/Lib/Light/CubeEnvironmentLight.h>
#include <Atrc/Lib/Light/SkyLight.h>
#include <Atrc/Mgr/BuiltinCreator/LightCreator.h>
#include <Atrc/Mgr/Parser.h>

namespace Atrc::Mgr
{

const Light *SkyLightCreator::Create(const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena)
{
    ATRC_MGR_TRY
    {
        auto top    = Parser::ParseSpectrum(group["top"]);
        auto bottom = Parser::ParseSpectrum(group["bottom"]);
        return arena.Create<SkyLight>(top, bottom);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating sky light: " + group.ToString())
}

const Light *CubeEnvironmentLightCreator::Create(const ConfigGroup &group, Context &context, Arena &arena)
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
        return arena.Create<CubeEnvironmentLight>(envTex);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating cube environment light: " + group.ToString())
}

} // namespace Atrc::Mgr
