#pragma once

#include <unordered_map>

#include <Atrc/Atrc.h>

using AGZ::Option;
using AGZ::Some;
using AGZ::None;

using AGZ::ObjArena;
using AGZ::Config;
using AGZ::ConfigGroup;
using AGZ::ConfigNode;

using AGZ::Str8;

template<typename T>
class ObjectCreator
{
public:

	virtual ~ObjectCreator() = default;

	virtual Str8 GetName() const = 0;

	virtual T *Create(const ConfigGroup &params, ObjArena<> &arena) const = 0;
};

template<typename T>
class ObjectManager : public AGZ::Singleton<ObjectManager<T>>
{
public:

	using Creator = ObjectCreator<T>;

	void AddCreator(const Creator *creator)
	{
		AGZ_ASSERT(creator);
		name2Creator_[creator->GetName()] = creator;
	}

    T *Create(const ConfigGroup &params, ObjArena<> &arena) const;

private:

	std::unordered_map<Str8, const Creator*> name2Creator_;
};

template<typename T>
class PublicDefinitionManager : public AGZ::Singleton<PublicDefinitionManager<T>>
{
    std::unordered_map<Str8, T*> definitions_;

public:

    void Initialize(const ConfigGroup &contents, ObjArena<> &arena)
    {
        for(auto it : contents.GetChildren())
            definitions_[it.first] = ObjectManager<T>::GetInstance().Create(it.second->AsGroup(), arena);
    }

    T *Get(const Str8 &name) const
    {
        auto it = definitions_.find(name);
        return it != definitions_.end() ? it->second : nullptr;
    }
};

template<typename T>
T *ObjectManager<T>::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto it = name2Creator_.find(params["type"].AsValue());
    if(it == name2Creator_.end())
        return nullptr;
    return it->second->Create(params, arena);
}

template<typename T>
T *GetSceneObject(const ConfigNode &node, ObjArena<> &arena)
{
    if(node.IsValue())
    {
        auto &n = node.AsValue();
        if(n.StartsWith("$"))
            return PublicDefinitionManager<T>::GetInstance().Get(n.Slice(1));
    }
    return ObjectManager<T>::GetInstance().Create(node.AsGroup(), arena);
}
