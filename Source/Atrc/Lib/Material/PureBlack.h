#pragma once

#include <Atrc/Lib/Core/Material.h>

namespace Atrc
{

class PureBlack : public Material
{
public:

    ShadingPoint GetShadingPoint(const Intersection &inct, Arena &arena) const noexcept override;
};

inline const PureBlack STATIC_PURE_BLACK;

} // namespace Atrc
