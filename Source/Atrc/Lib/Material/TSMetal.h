#pragma once

#include <Atrc/Lib/Core/Material.h>
#include <Atrc/Lib/Core/Texture.h>
#include <Atrc/Lib/Material/Utility/NormalMapper.h>

namespace Atrc
{
    
// 基于Torrance-Sparrow模型的金属材质
class TSMetal : public Material
{
    Spectrum etaI_, etaT_, k_;
    const Texture *rc_;
    const Texture *roughness_;

    const NormalMapper *normalMapper_;

public:

    TSMetal(
        const Spectrum &etaI, const Spectrum &etaT, const Spectrum &k,
        const Texture *rc, const Texture *roughness, const NormalMapper *normalMapper) noexcept;

    ShadingPoint GetShadingPoint(const Intersection &inct, Arena &arena) const override;
};

} // namespace Atrc
