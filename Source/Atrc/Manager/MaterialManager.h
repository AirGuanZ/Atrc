#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class MaterialParser
{
public:

    virtual ~MaterialParser() = default;

    virtual Material *Create(const SceneParamGroup &params, AGZ::ObjArena<> &arena) const = 0;
};

class MaterialManager
{
    std::unordered_map<Str8, const MaterialParser*> name2Mat_;

public:

    void SetParser(const Str8 &name, const MaterialParser *parser);

    Material *Create(const Str8 &name, const SceneParamGroup &params, AGZ::ObjArena<> &arena);
};

AGZ_NS_END(Atrc)
