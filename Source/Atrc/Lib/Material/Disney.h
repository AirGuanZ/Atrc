#pragma once

#include <Atrc/Lib/Core/Material.h>
#include <Atrc/Lib/Core/Texture.h>
#include <Atrc/Lib/Material/Utility/NormalMapper.h>

namespace Atrc
{
    
class DisneyPrincipledMaterial : public Material
{
    const Texture *rc_;
    const Texture *roughness_;
    const Texture *specular_;
    const Texture *specularTint_;
    const Texture *metallic_;
    const Texture *sheen_;
    const Texture *sheenTint_;
    const Texture *subsurface_;
    const Texture *clearCoat_;
    const Texture *clearCoatGloss_;

    const NormalMapper *normalMapper_;

public:

    DisneyPrincipledMaterial(
        const Texture *rc,
        const Texture *roughness,
        const Texture *specular,
        const Texture *specularTint,
        const Texture *metallic,
        const Texture *sheen,
        const Texture *sheenTint,
        const Texture *subsurface,
        const Texture *clearCoat,
        const Texture *clearCoatGloss,
        const NormalMapper *normalMapper) noexcept;

    ShadingPoint GetShadingPoint(const Intersection &inct, Arena &arena) const override;
};

} // namespace Atrc
