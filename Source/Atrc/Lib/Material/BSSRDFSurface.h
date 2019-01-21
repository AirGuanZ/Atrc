#pragma once

#include <Atrc/Lib/Core/Material.h>
#include <Atrc/Lib/Core/Texture.h>

namespace Atrc
{

class BSSRDFSurface : public Material
{
    const Material *surface_;

    const Texture *AMap_;
    const Texture *dmfpMap_;

    Real eta_;

public:

    BSSRDFSurface(const Material *surface, const Texture *AMap, const Texture *dmfpMap, Real eta) noexcept;

    ShadingPoint GetShadingPoint(const Intersection &inct, Arena &arena) const noexcept;
};

} // namespace Atrc
