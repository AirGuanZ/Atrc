#pragma once

#include <Atrc/Lib/Core/Material.h>

namespace Atrc
{

class InvisibleSurface : public Material
{
public:

    ShadingPoint GetShadingPoint(const Intersection &inct, Arena &arena) const override;
};

} // namespace Atrc
