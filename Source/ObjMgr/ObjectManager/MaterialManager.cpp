#include "../ParamParser.h"
#include "MaterialManager.h"
#include "TextureManager.h"

AGZ_NS_BEG(ObjMgr)

FresnelType FresnelCreator::Name2Type(const Str8 &name)
{
    if(name == "FresnelConductor")
        return FresnelType::FresnelConductor;
    if(name == "FresnelDielectric")
        return FresnelType::FresnelDielectric;
    if(name == "SchlickApproximation")
        return FresnelType::SchlickApproximation;
    throw SceneInitializationException("FresnelCreator: unknown fresnel type");
}

const Atrc::Fresnel *FresnelCreator::CreateFresnel(const FresnelType &type, const ConfigGroup &params, ObjArena<>& arena)
{
    if(type == FresnelType::FresnelConductor)
        return CreateFresnelConductor(params, arena);
    return CreateDielectric(type, params, arena);
}

const Atrc::Dielectric *FresnelCreator::CreateDielectric(const FresnelType &type, const ConfigGroup &params, ObjArena<> &arena)
{
    switch(type)
    {
    case FresnelType::FresnelDielectric:
        return CreateFresnelDielectric(params, arena);
    case FresnelType::SchlickApproximation:
        return CreateSchlickApproximation(params, arena);
    default:
        throw SceneInitializationException("FresnelCreator: unknown dielectric type");
    }
}

const Atrc::FresnelConductor *FresnelCreator::CreateFresnelConductor(const ConfigGroup &params, ObjArena<> &arena)
{
    auto etaI = ParamParser::ParseSpectrum(params["etaI"]);
    auto etaT = ParamParser::ParseSpectrum(params["etaT"]);
    auto k = ParamParser::ParseSpectrum(params["k"]);
    return arena.Create<Atrc::FresnelConductor>(etaI, etaT, k);
}

const Atrc::FresnelDielectric *FresnelCreator::CreateFresnelDielectric(const ConfigGroup &params, ObjArena<> &arena)
{
    auto etaI = params["etaI"].AsValue().Parse<float>();
    auto etaT = params["etaT"].AsValue().Parse<float>();
    return arena.Create<Atrc::FresnelDielectric>(etaI, etaT);
}

const Atrc::SchlickApproximation *FresnelCreator::CreateSchlickApproximation(const ConfigGroup &params, ObjArena<> &arena)
{
    auto etaI = params["etaI"].AsValue().Parse<float>();
    auto etaT = params["etaT"].AsValue().Parse<float>();
    return arena.Create<Atrc::SchlickApproximation>(etaI, etaT);
}

const Atrc::NormalMapper* NormalMapperCreator::CreateNormalMapper(const ConfigGroup &params, ObjArena<> &arena)
{
    auto node = params.Find("normalMap");
    if(!node)
    {
        static Atrc::TrivialNormalMapper trivialNorMap;
        return &trivialNorMap;
    }

    auto tex = GetSceneObject<Atrc::Texture>(*node, arena);
    if(!tex)
        throw SceneInitializationException("Failed to initialize texture for normal mapping");

    return arena.Create<Atrc::TextureNormalMapper>(tex);
}

Atrc::Material *BlackMaterialCreator::Create([[maybe_unused]] const ConfigGroup &params, [[maybe_unused]] ObjArena<> &arena) const
{
    return &Atrc::STATIC_BLACK_MATERIAL;
}

Atrc::Material *DiffuseMaterialCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto albedo = ParamParser::ParseSpectrum(params["albedo"]);
    auto norMap = NormalMapperCreator::CreateNormalMapper(params, arena);
    return arena.Create<Atrc::DiffuseMaterial>(albedo, norMap);
}

Atrc::Material *FresnelSpecularCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto rc = ParamParser::ParseSpectrum(params["rc"]);

    auto fresnelTypeName = params["fresnel.type"].AsValue();
    auto fresnelType = FresnelCreator::Name2Type(fresnelTypeName);
    auto fresnel = FresnelCreator::CreateDielectric(fresnelType, params["fresnel"].AsGroup(), arena);

    return arena.Create<Atrc::FresnelSpecular>(rc, fresnel);
}

Atrc::Material *IdealMirrorCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto rc = ParamParser::ParseSpectrum(params["rc"]);

    auto &fresnelTypeName = params["fresnel.type"].AsValue();
    auto fresnelType = FresnelCreator::Name2Type(fresnelTypeName);
    auto fresnel = FresnelCreator::CreateFresnel(fresnelType, params["fresnel"].AsGroup(), arena);

    return arena.Create<Atrc::IdealMirror>(rc, fresnel);
}

Atrc::Material* MetalCreator::Create(const ConfigGroup& params, ObjArena<>& arena) const
{
    auto rc   = ParamParser::ParseSpectrum(params["rc"]);
    auto etaI = ParamParser::ParseSpectrum(params["etaI"]);
    auto etaT = ParamParser::ParseSpectrum(params["etaT"]);;
    auto k    = ParamParser::ParseSpectrum(params["k"]);;
    auto roughness = params["roughness"].AsValue().Parse<Atrc::Real>();

    return arena.Create<Atrc::Metal>(rc, etaI, etaT, k, roughness);
}

Atrc::Material *PlasticCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto kd = ParamParser::ParseSpectrum(params["kd"]);
    auto ks = ParamParser::ParseSpectrum(params["ks"]);
    auto roughness = params["roughness"].AsValue().Parse<Atrc::Real>();

    return arena.Create<Atrc::Plastic>(kd, ks, roughness);
}

Atrc::Material *TextureScalerCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto tex = GetSceneObject<Atrc::Texture>(params["texture"], arena);
    auto mat = GetSceneObject<Atrc::Material>(params["internal"], arena);

    if(!tex)
        throw SceneInitializationException("TextureScalerCreator: failed to create texture object");
    if(!mat)
        throw SceneInitializationException("TextureScalerCreator: failed to create internal material object");

    return arena.Create<Atrc::TextureScaler>(tex, mat);
}

Atrc::Material *UncallableMaterialCreator::Create([[maybe_unused]] const ConfigGroup &params, [[maybe_unused]] ObjArena<> &arena) const
{
    return &Atrc::STATIC_UNCALLABLE_MATERIAL;
}

AGZ_NS_END(ObjMgr)
