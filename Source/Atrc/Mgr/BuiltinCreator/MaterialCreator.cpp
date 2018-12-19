#include <Utils/FileSys.h>
#include <Utils/Serialize.h>

#include <Atrc/Lib/Material/IdealBlack.h>
#include <Atrc/Lib/Material/IdealDiffuse.h>
#include <Atrc/Lib/Material/IdealMirror.h>
#include <Atrc/Lib/Material/InvisibleSurface.h>
#include <Atrc/Lib/Material/ONMatte.h>
#include <Atrc/Lib/Material/TSMetal.h>
#include <Atrc/Lib/Material/Utility/Fresnel.h>
#include <Atrc/Mgr/BuiltinCreator/MaterialCreator.h>
#include <Atrc/Mgr/Parser.h>

namespace Atrc::Mgr
{

namespace
{
    class FresnelConductorCreator : public Creator<Fresnel>
    {
    public:

        Str8 GetTypeName() const override { return "Conductor"; }

        Fresnel *Create(const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const override
        {
            ATRC_MGR_TRY
            {
                auto etaI = Parser::ParseSpectrum(group["etaI"]);
                auto etaT = Parser::ParseSpectrum(group["etaT"]);
                auto k = Parser::ParseSpectrum(group["k"]);
                return arena.Create<FresnelConductor>(etaI, etaT, k);
            }
            ATRC_MGR_CATCH_AND_RETHROW("In creating conductor fresnel object: " + group.ToString())
        }
    };

    class FresnelDielectricCreator : public Creator<Fresnel>
    {
    public:

        Str8 GetTypeName() const override { return "Dielectric"; }

        Fresnel *Create(const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const override
        {
            ATRC_MGR_TRY
            {
                Real etaI = group["etaI"].Parse<Real>();
                Real etaT = group["etaT"].Parse<Real>();
                return arena.Create<FresnelDielectric>(etaI, etaT);
            }
            ATRC_MGR_CATCH_AND_RETHROW("In creating dielectric object: " + group.ToString())
        }
    };

    class SchlickApproximationCreator : public Creator<Fresnel>
    {
    public:

        Str8 GetTypeName() const override { return "Schlick"; }

        Fresnel *Create(const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const override
        {
            ATRC_MGR_TRY
            {
                Real etaI = group["etaI"].Parse<Real>();
                Real etaT = group["etaT"].Parse<Real>();
                return arena.Create<SchlickApproximation>(etaI, etaT);
            }
            ATRC_MGR_CATCH_AND_RETHROW("In creating dielectric object: " + group.ToString())
        }
    };
} // namespace null

void RegisterBuiltinMaterialCreators(Context &context)
{
    static const IdealBlackCreator idealBlackCreator;
    static const IdealDiffuseCreator idealDiffuseCreator;
    static const IdealMirrorCreator idealMirrorCreator;
    static const InvisibleSurfaceCreator invisibleSurfaceCreator;
    static const ONMatteCreator oNMatteCreator;
    static const TSMetalCreator tSMetalCreator;
    context.AddCreator(&idealBlackCreator);
    context.AddCreator(&idealDiffuseCreator);
    context.AddCreator(&idealMirrorCreator);
    context.AddCreator(&invisibleSurfaceCreator);
    context.AddCreator(&oNMatteCreator);
    context.AddCreator(&tSMetalCreator);

    static const FresnelConductorCreator fresnelConductorCreator;
    static const FresnelDielectricCreator fresnelDielectricCreator;
    static const SchlickApproximationCreator schlickApproximationCreator;
    context.AddCreator(&fresnelConductorCreator);
    context.AddCreator(&fresnelDielectricCreator);
    context.AddCreator(&schlickApproximationCreator);
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

Material *IdealMirrorCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        auto rcMap = context.Create<Texture>(group["rc"]);
        auto fresnel = context.Create<Fresnel>(group["fresnel"]);
        return arena.Create<IdealMirror>(rcMap, fresnel);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating ideal mirror material: " + group.ToString())
}

Material *InvisibleSurfaceCreator::Create(
    [[maybe_unused]] const ConfigGroup &group, [[maybe_unused]] Context &context, [[maybe_unused]] Arena &arena) const
{
    static InvisibleSurface STATIC_INVISIBLE_SURFACE;
    return &STATIC_INVISIBLE_SURFACE;
}

Material *ONMatteCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        auto albedoMap    = context.Create<Texture>(group["albedo"]);
        auto sigmaMap     = context.Create<Texture>(group["sigma"]);
        auto normalMapper = CreateNormalMapper(group, context, arena);

        return arena.Create<ONMatte>(albedoMap, sigmaMap, normalMapper);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating Oren-Nayar matte material: " + group.ToString())
}

Material *TSMetalCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        auto fresnel = context.Create<Fresnel>(group["fresnel"]);
        auto rc = context.Create<Texture>(group["rc"]);
        auto roughness = context.Create<Texture>(group["roughness"]);
        auto normalMapper = CreateNormalMapper(group, context, arena);

        return arena.Create<TSMetal>(fresnel, rc, roughness, normalMapper);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating Torrance-Sparrow metal material: " + group.ToString())
}

} // namespace Atrc::Mgr
