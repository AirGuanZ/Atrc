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

// TODO

AGZ_NS_END(Atrc)
