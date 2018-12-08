#include "../ParamParser.h"
#include "LightManager.h"

AGZ_NS_BEG(ObjMgr)

namespace
{
    auto LoadTexture(const ConfigNode &params, ObjArena<> &arena)
    {
        auto ret = GetSceneObject<Atrc::Texture>(params, arena);
        if(!ret)
            throw SceneInitializationException("Failed to create texture object for light source");
        return ret;
    }
}

Atrc::Light *CubeEnvironmentLightCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto posX = LoadTexture(params["posX"], arena);
    auto posY = LoadTexture(params["posY"], arena);
    auto posZ = LoadTexture(params["posZ"], arena);
    auto negX = LoadTexture(params["negX"], arena);
    auto negY = LoadTexture(params["negY"], arena);
    auto negZ = LoadTexture(params["negZ"], arena);
    const Atrc::Texture *texArr[] = { posX, posY, posZ, negX, negY, negZ };
    return arena.Create<Atrc::CubeEnvironmentLight>(texArr);
}

Atrc::Light *DirectionalLightCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto direction = ParamParser::ParseVec3(params["direction"]);
    auto radiance = ParamParser::ParseSpectrum(params["radiance"]);

    return arena.Create<Atrc::DirectionalLight>(direction, radiance);
}

Atrc::Light *SkyLightCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto top    = ParamParser::ParseSpectrum(params["top"]);
    auto bottom = ParamParser::ParseSpectrum(params["bottom"]);

    return arena.Create<Atrc::SkyLight>(top, bottom);
}

Atrc::Light *SphereEnvironmentLightCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto tex = LoadTexture(params["tex"], arena);
    Atrc::Transform transform;
    auto transNode = params.Find("transform");
    if(transNode)
        transform = ParamParser::ParseTransform(*transNode);
    return arena.Create<Atrc::SphereEnvironmentLight>(tex, transform);
}

AGZ_NS_END(ObjMgr)
