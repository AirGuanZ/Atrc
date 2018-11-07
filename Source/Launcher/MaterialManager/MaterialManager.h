#pragma once

#include <unordered_map>

#include "../Common.h"
#include "MaterialCreator.h"

// MaterialDefinition:
// {
//		type = ...(material name)
//		...params
// }
class MaterialManager : public AGZ::Singleton<MaterialManager>
{
	ObjArena<> arena_;
	std::unordered_map<Str8, const MaterialCreator*> name2Creator_;

public:

	template<typename T, typename...Args>
	void AddCreator(const Str8 &name, Args&&...args)
	{
		AGZ_ASSERT(!name.Empty());
		auto creator = arena_.Create<T>(std::forward<Args>(args)...);
		name2Creator_[name] = creator;
	}

	const Atrc::Material *Create(const ConfigGroup &params)
	{
		auto it = name2Creator_.find(params["type"].AsValue());
		return it != name2Creator_.end() ? it->second->Create(params, arena_) : nullptr;
	}
};
