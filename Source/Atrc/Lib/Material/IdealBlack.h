#pragma once

#include <Atrc/Lib/Core/Material.h>

namespace Atrc
{

class IdealBlack : public Material
{
public:

    ShadingPoint GetShadingPoint(const Intersection &inct, Arena &arena) const noexcept override;
};

inline const IdealBlack STATIC_IDEAL_BLACK;

} // namespace Atrc
