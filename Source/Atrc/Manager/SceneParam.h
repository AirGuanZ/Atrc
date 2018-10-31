#pragma once

#include <map>

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

DEFINE_ATRC_EXCEPTION(SceneParamException_KeyNotFound);

class SceneParamGroup
{
    using Pair = std::pair<Option<Str8>, Option<SceneParamGroup>>;

    std::map<Str8, Pair> params_;

public:

    void Set(const Str8 &k, const Str8 &v);

    void Set(const Str8 &k, SceneParamGroup &&v);

    bool Find(const Str8 &k) const;

    // 找不到的话会抛SceneParamException_KeyNotFound
    const Str8 &GetValue(const Str8 &k) const;

    const SceneParamGroup &GetGroup(const Str8 &k) const;

    float GetFloat(const Str8 &k) const;

    Real GetReal(const Str8 &k) const;

    Spectrum GetSpectrum(const Str8 &k) const;

    Vec3 GetVec3(const Str8 &k) const;
};

AGZ_NS_END(Atrc)
