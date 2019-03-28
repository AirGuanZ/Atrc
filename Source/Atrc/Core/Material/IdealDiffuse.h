#pragma once

#include <Atrc/Core/Core/Material.h>
#include <Atrc/Core/Core/Texture.h>
#include <Atrc/Core/Material/Utility/NormalMapper.h>

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
