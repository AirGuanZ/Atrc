#pragma once

#include <Atrc/Lib/Core/Material.h>
#include <Atrc/Lib/Core/Texture.h>
#include <Atrc/Lib/Material/Utility/NormalMapper.h>

namespace Atrc
{

class IdealDiffuse : public Material
{
    const Texture *albedoMap_;
    const NormalMapper *normalMapper_;

public:

    IdealDiffuse(const Texture *albedoMap, const NormalMapper *normalMapper) noexcept;

    ShadingPoint GetShadingPoint(const Intersection &inct, Arena &arena) const noexcept override;
};

} // namespace Atrc
