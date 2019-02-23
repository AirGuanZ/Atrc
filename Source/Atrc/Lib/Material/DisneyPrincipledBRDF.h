#pragma once

#include <Atrc/Lib/Core/Material.h>
#include <Atrc/Lib/Core/Texture.h>
#include <Atrc/Lib/Material/Utility/NormalMapper.h>

namespace Atrc
{

// See https://airguanz.github.io/2019/02/20/disney-brdf.html
class DisneyBRDFMaterial : public Material
{
    const Texture *baseColor_;

    const Texture *subsurface_;
    const Texture *metallic_;
    const Texture *specular_;
    const Texture *specularTint_;
    const Texture *roughness_;
    const Texture *anisotropic_;
    const Texture *sheen_;
    const Texture *sheenTint_;
    const Texture *clearcoat_;
    const Texture *clearcoatGloss_;

    const NormalMapper *normalMapper_;

public:

    DisneyBRDFMaterial(
        const Texture *baseColor,
        const Texture *subsurface,
        const Texture *metallic,
        const Texture *specular,
        const Texture *specularTint,
        const Texture *roughness,
        const Texture *anisotropic,
        const Texture *sheen,
        const Texture *sheenTint,
        const Texture *clearcoat,
        const Texture *clearcoatGloss,
        const NormalMapper *normalMapper) noexcept;

    ShadingPoint GetShadingPoint(const Intersection &inct, Arena &arena) const override;
};

} // namespace Atrc
