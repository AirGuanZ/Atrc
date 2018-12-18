#pragma once

#include <Atrc/Lib/Core/Material.h>
#include <Atrc/Lib/Material/Utility/NormalMapper.h>

namespace Atrc
{
    
class ONMatte : public Material
{
    const Texture *albedoMap_;
    const Texture *sigmaMap_;
    const NormalMapper *normalMapper_;

public:

    ONMatte(const Texture *albedoMap, const Texture *sigmaMap, const NormalMapper *normalMapper) noexcept;

    ShadingPoint GetShadingPoint(const Intersection &inct, Arena &arena) const override;
};

} // namespace Atrc
