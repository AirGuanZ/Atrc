#pragma once

#include <Atrc/Core/Core.h>
#include <Atrc/Material/BxDF/TorranceSparrow/BlinnPhongDistribution.h>
#include <Atrc/Material/BxDF/TorranceSparrow.h>
#include <Atrc/Material/Fresnel.h>

AGZ_NS_BEG(Atrc)

class Plastic : public Material
{
    Spectrum kd_;

    FresnelDielectric fresnel_;
    BlinnPhongDistribution md_;
    TorranceSparrow torranceSparrow_;

public:

    Plastic(const Spectrum &kd, const Spectrum &ks, Real roughness);

    void Shade(const SurfacePoint &sp, ShadingPoint *dst, AGZ::ObjArena<> &arena) const override;
};

AGZ_NS_END(Atrc)
