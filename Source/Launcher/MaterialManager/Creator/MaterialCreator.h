#pragma once

#include "../Common.h"

class MaterialCreator
{
public:

	virtual ~MaterialCreator() = default;

	virtual const Atrc::Material *Create(const ConfigGroup &params, ObjArena<> &arena) const = 0;
};
