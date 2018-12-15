#include <Atrc/Lib/Material/IdealBlack.h>
#include <Atrc/Lib/Material/IdealDiffuse.h>
#include <Atrc/Lib/Material/TSMetal.h>
#include <Atrc/Mgr/BuiltinCreator/MaterialCreator.h>
#include <Atrc/Mgr/Parser.h>

namespace Atrc::Mgr
{

void RegisterBuiltinMaterialCreators(Context &context)
{
    static const IdealBlackCreator idealBlackCreator;
    static const IdealDiffuseCreator idealDiffuseCreator;
    static const TSMetalCreator tSMetalCreator;
    context.AddCreator(&idealBlackCreator);
    context.AddCreator(&idealDiffuseCreator);
    context.AddCreator(&tSMetalCreator);
}

namespace
{
    // 若group.Find("normalMapper")不为nullptr，则以之为texture创建一个texture normal mapper，否则返回default normal mapper
    const NormalMapper *CreateNormalMapper(const ConfigGroup &group, Context &context, Arena &arena)
    {
        ATRC_MGR_TRY
        {
            if(auto texNode = group.Find("normalMapper"))
            {
                auto tex = context.Create<Texture>(*texNode);
                return arena.Create<TextureNormalMapper>(tex);
            }
            static const DefaultNormalMapper DEFAULT_NORMAL_MAPPER;
            return &DEFAULT_NORMAL_MAPPER;
        }
        ATRC_MGR_CATCH_AND_RETHROW("In creating normal mapper: " + group.ToString())
    }
}

Material *IdealBlackCreator::Create(
    [[maybe_unused]] const ConfigGroup &group, [[maybe_unused]] Context &context, [[maybe_unused]] Arena &arena) const
{
    return &STATIC_IDEAL_BLACK;
}

Material *IdealDiffuseCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        auto albedoMap = context.Create<Texture>(group["albedo"]);
        auto normalMapper = CreateNormalMapper(group, context, arena);
        return arena.Create<IdealDiffuse>(albedoMap, normalMapper);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating ideal diffuse material: " + group.ToString())
}

Material *TSMetalCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        auto etaI = Parser::ParseSpectrum(group["etaI"]);
        auto etaT = Parser::ParseSpectrum(group["etaT"]);
        auto k    = Parser::ParseSpectrum(group["k"]);

        auto rc        = context.Create<Texture>(group["rc"]);
        auto roughness = context.Create<Texture>(group["roughness"]);

        auto normalMapper = CreateNormalMapper(group, context, arena);

        return arena.Create<TSMetal>(etaI, etaT, k, rc, roughness, normalMapper);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating torrance sparrow metal material: " + group.ToString())
}

} // namespace Atrc::Mgr
