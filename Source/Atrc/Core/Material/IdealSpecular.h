#pragma once

#include <Atrc/Core/Core/Material.h>
#include <Atrc/Core/Material/Utility/Fresnel.h>

namespace Atrc
{

class Texture;

class IdealSpecular : public Material
{
    const Texture *rcMap_;
    const Dielectric *fresnel_;

public:

    IdealSpecular(const Texture *rcMap, const Fresnel *fresnel) noexcept;

    ShadingPoint GetShadingPoint(const Intersection &inct, Arena &arena) const override;
};

} // namespace Atrc

