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

	T *Create(const ConfigGroup &params, ObjArena<> &arena) const
	{
		auto it = name2Creator_.find(params["type"].AsValue());
		return it != name2Creator_.end() ? it->second->Create(params, arena) : nullptr;
	}

private:

	std::unordered_map<Str8, const Creator*> name2Creator_;
};
