#pragma once

#include <Atrc/Core/Material.h>

AGZ_NS_BEG(Atrc)

class PureDiffuse : public Material
{
    Spectrum color_;

public:

    explicit PureDiffuse(const Spectrum &albedo);

    void Shade(const SurfacePoint &sp, ShadingPoint *dst) const override;
};

AGZ_NS_END(Atrc)
