#pragma once

#include <Atrc/Core/Core/Material.h>
#include <Atrc/Core/Core/Texture.h>

namespace Atrc
{

class IdealScaler : public Material
{
    const Texture *scaleMap_;
    const Material *internal_;

public:

    IdealScaler(const Texture *scaleMap, const Material *internal) noexcept;

    ShadingPoint GetShadingPoint(const Intersection &inct, Arena &arena) const noexcept override;
};

} // namespace Atrc
