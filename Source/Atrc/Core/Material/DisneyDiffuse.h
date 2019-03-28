#pragma once

#include <Atrc/Core/Core/Material.h>
#include <Atrc/Core/Core/Texture.h>
#include <Atrc/Core/Material/Utility/NormalMapper.h>

namespace Atrc
{

// See https://airguanz.github.io/2019/02/20/disney-brdf.html
class DisneyDiffuseMaterial : public Material
{
    const Texture *baseColor_;

    const Texture *subsurface_;
    const Texture *roughness_;

    const NormalMapper *normalMapper_;

public:

    DisneyDiffuseMaterial(
        const Texture *baseColor,
        const Texture *subsurface,
        const Texture *roughness,
        const NormalMapper *normalMapper) noexcept;

    ShadingPoint GetShadingPoint(const Intersection &inct, Arena &arena) const override;
};

} // namespace Atrc
