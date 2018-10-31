#include <Atrc/Manager/SceneParam.h>
#include <Atrc/Utility/ParamParser.h>

AGZ_NS_BEG(Atrc)

void SceneParamGroup::Set(const Str8 &k, const Str8 &v)
{
    auto &it = params_[k];
    it.first = v;
    it.second = None;
}

void SceneParamGroup::Set(const Str8 &k, SceneParamGroup &&v)
{
    auto &it = params_[k];
    it.first = None;
    it.second = std::move(v);
}

bool SceneParamGroup::Find(const Str8 &k) const
{
    return params_.find(k) != params_.end();
}

const Str8 &SceneParamGroup::GetValue(const Str8 &k) const
{
    auto it = params_.find(k);
    if(it == params_.end())
        throw SceneParamException_KeyNotFound(("Key not found: " + k).ToStdString());
    if(!it->second.first.has_value())
        throw SceneParamException_KeyNotFound(("Key with wrong type: " + k).ToStdString());
    return *it->second.first;
}

const SceneParamGroup &SceneParamGroup::GetGroup(const Str8 &k) const
{
    auto it = params_.find(k);
    if(it == params_.end())
        throw SceneParamException_KeyNotFound(("Key not found: " + k).ToStdString());
    if(!it->second.second.has_value())
        throw SceneParamException_KeyNotFound(("Key with wrong type: " + k).ToStdString());
    return *it->second.second;
}

float SceneParamGroup::GetFloat(const Str8 &k) const
{
    return ParamParser::ParseFloat(GetValue(k));
}

Real SceneParamGroup::GetReal(const Str8 &k) const
{
    return ParamParser::ParseReal(GetValue(k));
}

Spectrum SceneParamGroup::GetSpectrum(const Str8 &k) const
{
    return ParamParser::ParseRGB(GetValue(k));
}

Vec3 SceneParamGroup::GetVec3(const Str8 &k) const
{
    return ParamParser::ParseVec3(GetValue(k));
}

AGZ_NS_END(Atrc)
