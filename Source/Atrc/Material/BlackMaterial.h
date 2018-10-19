#pragma once

#include <Atrc/Core/Common.h>
#include <Atrc/Core/Material.h>

AGZ_NS_BEG(Atrc)

class BlackMaterial : public Material
{
public:

    void Shade(const SurfacePoint &sp, ShadingPoint *dst) const override;
};

AGZ_NS_END(Atrc)
