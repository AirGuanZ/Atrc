#pragma once

#include <Atrc/Lib/Core/Material.h>
#include <Atrc/Lib/Core/Texture.h>
#include <Atrc/Lib/Material/Utility/NormalMapper.h>

namespace Atrc
{

class Fresnel;
    
// 基于Torrance-Sparrow模型的金属材质
class TSMetal : public Material
{
    const Fresnel *fresnel_;

    const Texture *rc_;
    const Texture *roughness_;

    const NormalMapper *normalMapper_;

public:

    TSMetal(
        const Fresnel *fresnel,
        const Texture *rc, const Texture *roughness, const NormalMapper *normalMapper) noexcept;

    ShadingPoint GetShadingPoint(const Intersection &inct, Arena &arena) const override;
};

} // namespace Atrc
