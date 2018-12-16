#pragma once

#include <Atrc/Lib/Core/Material.h>

namespace Atrc
{

class Fresnel;
class Texture;

class IdealMirror : public Material
{
    const Texture *rcMap_;
    const Fresnel *fresnel_;

public:

    IdealMirror(const Texture *rcMap, const Fresnel *fresnel) noexcept;

    ShadingPoint GetShadingPoint(const Intersection &inct, Arena &arena) const override;
};

} // namespace Atrc
