#include <Atrc/Core/Material/BSSRDFSurface.h>
#include <Atrc/Core/Material/DisneyDiffuse.h>
#include <Atrc/Core/Material/DisneyReflection.h>
#include <Atrc/Core/Material/DisneySpecular.h>
#include <Atrc/Core/Material/GGXDielectric.h>
#include <Atrc/Core/Material/GGXMetal.h>
#include <Atrc/Core/Material/IdealBlack.h>
#include <Atrc/Core/Material/IdealDiffuse.h>
#include <Atrc/Core/Material/IdealMirror.h>
#include <Atrc/Core/Material/IdealScaler.h>
#include <Atrc/Core/Material/IdealSpecular.h>
#include <Atrc/Core/Material/InvisibleSurface.h>
#include <Atrc/Core/Material/ONMatte.h>
#include <Atrc/Core/Material/TSMetal.h>
#include <Atrc/Core/Material/Utility/Fresnel.h>
#include <Atrc/Mgr/BuiltinCreator/MaterialCreator.h>
#include <Atrc/Mgr/Parser.h>

namespace Atrc::Mgr
{

namespace
{
    class AlwaysOneFresnelCreator : public Creator<Fresnel>
    {
    public:

        std::string GetTypeName() const override { return "AlwaysOne"; }

        Fresnel *Create(const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const override
        {
            AGZ_HIERARCHY_TRY
            {
                return arena.Create<AlwaysOneFresnel>();
            }
            AGZ_HIERARCHY_WRAP("In creating always-one fresnel object: " + group.ToString())
        }
    };

    class FresnelConductorCreator : public Creator<Fresnel>
    {
    public:

        std::string GetTypeName() const override { return "Conductor"; }

        Fresnel *Create(const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const override
        {
            AGZ_HIERARCHY_TRY
            {
                auto etaI = Parser::ParseSpectrum(group["etaI"]);
                auto etaT = Parser::ParseSpectrum(group["etaT"]);
                auto k = Parser::ParseSpectrum(group["k"]);
                return arena.Create<FresnelConductor>(etaI, etaT, k);
            }
            AGZ_HIERARCHY_WRAP("In creating conductor fresnel object: " + group.ToString())
        }
    };

    class FresnelDielectricCreator : public Creator<Fresnel>
    {
    public:

        std::string GetTypeName() const override { return "Dielectric"; }

        Fresnel *Create(const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const override
        {
            AGZ_HIERARCHY_TRY
            {
                Real etaI = group["etaI"].Parse<Real>();
                Real etaT = group["etaT"].Parse<Real>();
                return arena.Create<FresnelDielectric>(etaI, etaT);
            }
            AGZ_HIERARCHY_WRAP("In creating dielectric object: " + group.ToString())
        }
    };

    class SchlickApproximationCreator : public Creator<Fresnel>
    {
    public:

        std::string GetTypeName() const override { return "Schlick"; }

        Fresnel *Create(const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const override
        {
            AGZ_HIERARCHY_TRY
            {
                Real etaI = group["etaI"].Parse<Real>();
                Real etaT = group["etaT"].Parse<Real>();
                return arena.Create<SchlickApproximation>(etaI, etaT);
            }
            AGZ_HIERARCHY_WRAP("In creating dielectric object: " + group.ToString())
        }
    };
} // namespace null

void RegisterBuiltinMaterialCreators(Context &context)
{
    static const BSSRDFSurfaceCreator iBSSRDFSurfaceCreator;
    static const DisneyDiffuseCreator iDisneyDiffuseCreator;
    static const DisneyReflectionCreator iDisneyReflectionCreator;
    static const DisneySpecularCreator iDisneySpecularCreator;
    static const GGXDielectricCreator iGGXDielectricCreator;
    static const GGXMetalCreator iGGXMetalCreator;
    static const IdealBlackCreator idealBlackCreator;
    static const IdealDiffuseCreator idealDiffuseCreator;
    static const IdealMirrorCreator idealMirrorCreator;
    static const IdealScalerCreator idealScalerCreator;
    static const IdealSpecularCreator idealSpecularCreator;
    static const InvisibleSurfaceCreator invisibleSurfaceCreator;
    static const ONMatteCreator oNMatteCreator;
    static const TSMetalCreator tSMetalCreator;
    context.AddCreator(&iBSSRDFSurfaceCreator);
    context.AddCreator(&iDisneyDiffuseCreator);
    context.AddCreator(&iDisneyReflectionCreator);
    context.AddCreator(&iDisneySpecularCreator);
    context.AddCreator(&iGGXDielectricCreator);
    context.AddCreator(&iGGXMetalCreator);
    context.AddCreator(&idealBlackCreator);
    context.AddCreator(&idealDiffuseCreator);
    context.AddCreator(&idealMirrorCreator);
    context.AddCreator(&idealScalerCreator);
    context.AddCreator(&idealSpecularCreator);
    context.AddCreator(&invisibleSurfaceCreator);
    context.AddCreator(&oNMatteCreator);
    context.AddCreator(&tSMetalCreator);

    static const AlwaysOneFresnelCreator iAlwaysOneFresnelCreator;
    static const FresnelConductorCreator fresnelConductorCreator;
    static const FresnelDielectricCreator fresnelDielectricCreator;
    static const SchlickApproximationCreator schlickApproximationCreator;
    context.AddCreator(&iAlwaysOneFresnelCreator);
    context.AddCreator(&fresnelConductorCreator);
    context.AddCreator(&fresnelDielectricCreator);
    context.AddCreator(&schlickApproximationCreator);
}

namespace
{
    // 若group.Find("normalMapper")不为nullptr，则以之为texture创建一个texture normal mapper，否则返回default normal mapper
    const NormalMapper *CreateNormalMapper(const ConfigGroup &group, Context &context, Arena &arena)
    {
        AGZ_HIERARCHY_TRY
        {
            if(auto texNode = group.Find("normalMapper"))
            {
                auto tex = context.Create<Texture>(*texNode);
                return arena.Create<TextureNormalMapper>(tex);
            }
            static const DefaultNormalMapper DEFAULT_NORMAL_MAPPER;
            return &DEFAULT_NORMAL_MAPPER;
        }
        AGZ_HIERARCHY_WRAP("In creating normal mapper: " + group.ToString())
    }
}

Material *BSSRDFSurfaceCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY
    {
        auto surface = context.Create<Material>(group["surface"]);
        auto AMap    = context.Create<Texture>(group["A"]);
        auto dmfpMap = context.Create<Texture>(group["dmfp"]);
        Real eta     = group["eta"].Parse<Real>();

        return arena.Create<BSSRDFSurface>(surface, AMap, dmfpMap, eta);
    }
    AGZ_HIERARCHY_WRAP("In creating bssrdf surface material: " + group.ToString())
}

Material *DisneyDiffuseCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY
    {
        auto baseColor = context.Create<Texture>(group["baseColor"]);
        auto subsurface = context.Create<Texture>(group["subsurface"]);
        auto roughness = context.Create<Texture>(group["roughness"]);
        auto normalMapper = CreateNormalMapper(group, context, arena);

        return arena.Create<DisneyDiffuseMaterial>(
            baseColor, subsurface, roughness, normalMapper);
    }
    AGZ_HIERARCHY_WRAP("In creating disney diffuse: " + group.ToString())
}

Material *DisneyReflectionCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY
    {
        auto baseColor = context.Create<Texture>(group["baseColor"]);
        auto metallic = context.Create<Texture>(group["metallic"]);
        auto subsurface = context.Create<Texture>(group["subsurface"]);
        auto specular = context.Create<Texture>(group["specular"]);
        auto specularTint = context.Create<Texture>(group["specularTint"]);
        auto roughness = context.Create<Texture>(group["roughness"]);
        auto anisotropic = context.Create<Texture>(group["anisotropic"]);
        auto sheen = context.Create<Texture>(group["sheen"]);
        auto sheenTint = context.Create<Texture>(group["sheenTint"]);
        auto clearcoat = context.Create<Texture>(group["clearcoat"]);
        auto clearcoatGloss = context.Create<Texture>(group["clearcoatGloss"]);

        auto normalMapper = CreateNormalMapper(group, context, arena);

        return arena.Create<DisneyReflection>(
            baseColor, metallic, subsurface, specular, specularTint,
            roughness, anisotropic, sheen, sheenTint,
            clearcoat, clearcoatGloss, normalMapper);
    }
    AGZ_HIERARCHY_WRAP("In creating disney brdf: " + group.ToString())
}

Material *DisneySpecularCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY
    {
        auto baseColor = context.Create<Texture>(group["baseColor"]);
        auto specular = context.Create<Texture>(group["specular"]);
        auto specularTint = context.Create<Texture>(group["specularTint"]);
        auto metallic = context.Create<Texture>(group["metallic"]);
        auto roughness = context.Create<Texture>(group["roughness"]);
        auto anisotropic = context.Create<Texture>(group["anisotropic"]);

        auto normalMapper = CreateNormalMapper(group, context, arena);

        return arena.Create<DisneySpecularMaterial>(
            baseColor, specular, specularTint, metallic, roughness, anisotropic, normalMapper);
    }
    AGZ_HIERARCHY_WRAP("In creating disney specular: " + group.ToString())
}

Material *GGXDielectricCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY
    {
        auto dielectric = dynamic_cast<Dielectric*>(context.Create<Fresnel>(group["fresnel"]));
        auto rc = context.Create<Texture>(group["rc"]);
        auto roughness = context.Create<Texture>(group["roughness"]);
        auto normalMapper = CreateNormalMapper(group, context, arena);

        return arena.Create<GGXDielectric>(dielectric, rc, roughness, normalMapper);
    }
    AGZ_HIERARCHY_WRAP("In creating ggx dielectric material: " + group.ToString())
}

Material *GGXMetalCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY
    {
        auto fresnel      = context.Create<Fresnel>(group["fresnel"]);
        auto rc           = context.Create<Texture>(group["rc"]);
        auto roughness    = context.Create<Texture>(group["roughness"]);
        auto normalMapper = CreateNormalMapper(group, context, arena);

        return arena.Create<GGXMetal>(fresnel, rc, roughness, normalMapper);
    }
    AGZ_HIERARCHY_WRAP("In creating ggx metal material: " + group.ToString())
}

Material *IdealBlackCreator::Create(
    [[maybe_unused]] const ConfigGroup &group, [[maybe_unused]] Context &context, [[maybe_unused]] Arena &arena) const
{
    return &STATIC_IDEAL_BLACK;
}

Material *IdealDiffuseCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY
    {
        auto albedoMap = context.Create<Texture>(group["albedo"]);
        auto normalMapper = CreateNormalMapper(group, context, arena);
        return arena.Create<IdealDiffuse>(albedoMap, normalMapper);
    }
    AGZ_HIERARCHY_WRAP("In creating ideal diffuse material: " + group.ToString())
}

Material *IdealMirrorCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY
    {
        auto rcMap = context.Create<Texture>(group["rc"]);
        auto fresnel = context.Create<Fresnel>(group["fresnel"]);
        return arena.Create<IdealMirror>(rcMap, fresnel);
    }
    AGZ_HIERARCHY_WRAP("In creating ideal mirror material: " + group.ToString())
}

Material *IdealScalerCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY
    {
        auto scaleMap = context.Create<Texture>(group["scale"]);
        auto internal = context.Create<Material>(group["internal"]);
        return arena.Create<IdealScaler>(scaleMap, internal);
    }
    AGZ_HIERARCHY_WRAP("In creating ideal scaler material: " + group.ToString())
}

Material *IdealSpecularCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY
    {
        auto rcMap = context.Create<Texture>(group["rc"]);
        auto fresnel = context.Create<Fresnel>(group["fresnel"]);
        return arena.Create<IdealSpecular>(rcMap, fresnel);
    }
    AGZ_HIERARCHY_WRAP("In creating ideal specular material: " + group.ToString())
}

Material *InvisibleSurfaceCreator::Create(
    [[maybe_unused]] const ConfigGroup &group, [[maybe_unused]] Context &context, [[maybe_unused]] Arena &arena) const
{
    static InvisibleSurface STATIC_INVISIBLE_SURFACE;
    return &STATIC_INVISIBLE_SURFACE;
}

Material *ONMatteCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY
    {
        auto albedoMap    = context.Create<Texture>(group["albedo"]);
        auto sigmaMap     = context.Create<Texture>(group["sigma"]);
        auto normalMapper = CreateNormalMapper(group, context, arena);

        return arena.Create<ONMatte>(albedoMap, sigmaMap, normalMapper);
    }
    AGZ_HIERARCHY_WRAP("In creating Oren-Nayar matte material: " + group.ToString())
}

Material *TSMetalCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY
    {
        auto fresnel = context.Create<Fresnel>(group["fresnel"]);
        auto rc = context.Create<Texture>(group["rc"]);
        auto roughness = context.Create<Texture>(group["roughness"]);
        auto normalMapper = CreateNormalMapper(group, context, arena);

        return arena.Create<TSMetal>(fresnel, rc, roughness, normalMapper);
    }
    AGZ_HIERARCHY_WRAP("In creating Torrance-Sparrow metal material: " + group.ToString())
}

} // namespace Atrc::Mgr
