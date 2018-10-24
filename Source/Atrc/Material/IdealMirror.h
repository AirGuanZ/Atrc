#pragma once

#include <Atrc/Core/Core.h>
#include <Atrc/Material/Fresnel.h>

AGZ_NS_BEG(Atrc)

class IdealMirror : public Material
{
    Spectrum rc_;
    RC<const Fresnel> fresnel_;

public:

    IdealMirror(const Spectrum &rc, RC<const Fresnel> fresnel);

    void Shade(const SurfacePoint &sp, ShadingPoint *dst) const override;
};

AGZ_NS_END(Atrc)
