#pragma once

#include <Atrc/Lib/Core/Material.h>
#include <Atrc/Lib/Core/Texture.h>
#include <Atrc/Lib/Material/Utility/NormalMapper.h>

namespace Atrc
{

// See https://airguanz.github.io/2019/02/20/disney-brdf.html
class DisneySpecularMaterial : public Material
{
    const Texture *baseColor_;
    const Texture *specular_;
    const Texture *specularTint_;
    const Texture *metallic_;
    const Texture *roughness_;
    const Texture *anisotropic_;

    const NormalMapper *normalMapper_;

public:

    DisneySpecularMaterial(
        const Texture *baseColor,
        const Texture *specular,
        const Texture *specularTint,
        const Texture *metallic,
        const Texture *roughness,
        const Texture *anisotropic,
        const NormalMapper *normalMapper) noexcept;

    ShadingPoint GetShadingPoint(const Intersection &inct, Arena &arena) const override;
};

} // namespace Atrc
