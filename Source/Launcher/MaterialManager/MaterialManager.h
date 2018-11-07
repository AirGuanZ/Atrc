#pragma once

#include <unordered_map>

#include "Creator/MaterialCreator.h"

class MatrialManager
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

	const Atrc::Material *Create(const Str8 &name, const ConfigGroup &params)
	{
		auto it = name2Creator_.find(name);
		return it != name2Creator_.end() ? it->second->Create(params, arena_) : nullptr;
	}
};
