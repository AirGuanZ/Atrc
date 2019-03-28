#pragma once

#include <Atrc/Core/Core/Material.h>
#include <Atrc/Core/Core/Texture.h>
#include <Atrc/Core/Material/Utility/NormalMapper.h>

namespace Atrc
{

class Fresnel;
    
// 基于Torrance-Sparrow模型粗糙导体材质，采用GGX microfacet distribution
class GGXMetal : public Material
{
    const Fresnel *fresnel_;

    const Texture *rc_;
    const Texture *roughness_;

    const NormalMapper *normalMapper_;

public:

    GGXMetal(
        const Fresnel *fresnel,
        const Texture *rc, const Texture *roughness, const NormalMapper *normalMapper) noexcept;

    ShadingPoint GetShadingPoint(const Intersection &inct, Arena &arena) const override;
};

} // namespace Atrc
