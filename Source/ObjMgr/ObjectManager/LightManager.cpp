#include "../ParamParser.h"
#include "LightManager.h"
#include "TextureManager.h"

AGZ_NS_BEG(ObjMgr)

namespace
{
    auto LoadTexture(const Str8 &filename)
    {
        auto ret = TextureManager::GetInstance().Load(filename);
        if(!ret)
            throw SceneInitializationException("Failed to load texture from: " + filename);
        return ret;
    }
}

Atrc::Light *CubeEnvironmentLightCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto posX = LoadTexture(params["posX"].AsValue());
    auto posY = LoadTexture(params["posY"].AsValue());
    auto posZ = LoadTexture(params["posZ"].AsValue());
    auto negX = LoadTexture(params["negX"].AsValue());
    auto negY = LoadTexture(params["negY"].AsValue());
    auto negZ = LoadTexture(params["negZ"].AsValue());
    const AGZ::Texture2D<Atrc::Spectrum> *texArr[] = { posX, posY, posZ, negX, negY, negZ };
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
    auto tex = LoadTexture(params["tex"].AsValue());
    return arena.Create<Atrc::SphereEnvironmentLight>(tex);
}

AGZ_NS_END(ObjMgr)
