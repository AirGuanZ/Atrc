#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class DiffuseMaterial : public Material
{
    Spectrum color_;

public:

    explicit DiffuseMaterial(const Spectrum &albedo);

    void Shade(const SurfacePoint &sp, ShadingPoint *dst) const override;
};

AGZ_NS_END(Atrc)
