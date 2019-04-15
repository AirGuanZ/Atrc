#include <Atrc/Core/Light/CubeEnvironmentLight.h>
#include <Atrc/Core/Light/EnvironmentLight.h>
#include <Atrc/Core/Light/SHEnvironment.h>
#include <Atrc/Core/Light/SkyLight.h>
#include <Atrc/Mgr/BuiltinCreator/LightCreator.h>
#include <Atrc/Mgr/Parser.h>

namespace Atrc::Mgr
{

void RegisterBuiltinLightCreators(Context &context)
{
    static const CubeEnvironmentLightCreator cubeEnvironmentLight;
    static const EnvironmentLightCreator environmentLightCreator;
    static const SHEnvLightCreator iSHEnvLightCreator;
    static const SkyLightCreator skyLightCreator;
    context.AddCreator(&cubeEnvironmentLight);
    context.AddCreator(&environmentLightCreator);
    context.AddCreator(&iSHEnvLightCreator);
    context.AddCreator(&skyLightCreator);
}

Light *CubeEnvironmentLightCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY
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
    AGZ_HIERARCHY_WRAP("In creating cube environment light: " + group.ToString())
}

Light *EnvironmentLightCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY
    {
        auto tex = context.Create<Texture>(group["tex"]);

        Transform transform;
        if(auto tNode = group.Find("transform"))
            transform = Parser::ParseTransform(*tNode);

        return arena.Create<EnvironmentLight>(tex, transform);
    }
    AGZ_HIERARCHY_WRAP("In creating environment light: " + group.ToString())
}

Light *SHEnvLightCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY

    int SHOrder = group["SHOrder"].Parse<int>();
    if(SHOrder < 1 || SHOrder > 5)
        throw Exception("invalid SHOrder value: " + std::to_string(SHOrder));

    int SHC = SHOrder * SHOrder;
    std::vector<Spectrum> coefs(SHC);
    auto &coefArr = group["coefs"].AsArray();
    for(int i = 0; i < SHC; ++i)
        coefs[i] = Parser::ParseSpectrum(*coefArr.At(i));

    Real rotateDeg = 0;
    if(auto *rotNode = group.Find("rotateAngle"))
        rotateDeg = Deg(Parser::ParseAngle(*rotNode)).value;

    return arena.Create<SHEnvLight>(SHOrder, coefs.data(), rotateDeg);

    AGZ_HIERARCHY_WRAP("in creating sh environment light: " + group.ToString())
}

Light *SkyLightCreator::Create(const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY
    {
        auto top    = Parser::ParseSpectrum(group["top"]);
        auto bottom = Parser::ParseSpectrum(group["bottom"]);
        
        Transform transform;
        if(auto tNode = group.Find("transform"))
            transform = Parser::ParseTransform(*tNode);

        return arena.Create<SkyLight>(top, bottom, transform);
    }
    AGZ_HIERARCHY_WRAP("In creating sky light: " + group.ToString())
}

} // namespace Atrc::Mgr
