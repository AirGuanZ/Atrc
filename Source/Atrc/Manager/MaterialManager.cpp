#include <Atrc/Manager/MaterialManager.h>

AGZ_NS_BEG(Atrc)

void MaterialManager::SetParser(const Str8 &name, const MaterialCreator *parser)
{
    AGZ_ASSERT(parser);
    name2Mat_[name] = parser;
}

Material *MaterialManager::Create(const Str8 &name, const SceneParamGroup &params, AGZ::ObjArena<> &arena)
{
    auto it = name2Mat_.find(name);
    if(it == name2Mat_.end())
        throw ArgumentException("Unknown material: " + name.ToStdString());
    return it->second->Create(params, arena);
}

FresnelConductor *CreateFresnelConductor(const SceneParamGroup &params, AGZ::ObjArena<> &arena)
{
    Spectrum etaI = params.GetSpectrum("etaI");
    Spectrum etaT = params.GetSpectrum("etaT");
    Spectrum k = params.GetSpectrum("k");
    return arena.Create<FresnelConductor>(etaI, etaT, k);
}

FresnelDielectric *CreateFresnelDielectric(const SceneParamGroup &params, AGZ::ObjArena<> &arena)
{
    float etaI = params.GetFloat("etaI");
    float etaT = params.GetFloat("etaT");
    return arena.Create<FresnelDielectric>(etaI, etaT);
}

SchlickApproximation *CreateSchlickApproximation(const SceneParamGroup &params, AGZ::ObjArena<> &arena)
{
    float etaI = params.GetFloat("etaI");
    float etaT = params.GetFloat("etaT");
    return arena.Create<SchlickApproximation>(etaI, etaT);
}

Fresnel *CreateFresnel(const SceneParamGroup &params, AGZ::ObjArena<> &arena)
{
    auto &s = params.GetValue("Fresnel");
    if(s == "FresnelConductor")
        return CreateFresnelConductor(params.GetGroup(s), arena);
    if(s == "FresnelDielectric")
        return CreateFresnelDielectric(params.GetGroup(s), arena);
    if(s == "SchlickApproximation")
        return CreateSchlickApproximation(params.GetGroup(s), arena);
    throw ArgumentException(("Unknown Fresnel type: " + s).ToStdString());
}

Dielectric *CreateDielectric(const SceneParamGroup &params, AGZ::ObjArena<> &arena)
{
    auto &s = params.GetValue("Dielectric");
    if(s == "FresnelDielectric")
        return CreateFresnelDielectric(params.GetGroup(s), arena);
    if(s == "SchlickApproximation")
        return CreateSchlickApproximation(params.GetGroup(s), arena);
    throw ArgumentException(("Unknown Dielectric type: " + s).ToStdString());
}

// TODO

AGZ_NS_END(Atrc)
