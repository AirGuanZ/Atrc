#pragma once

#include <Atrc/Core/Core.h>
#include <Atrc/Material/BxDF/TorranceSparrow/BlinnPhongDistribution.h>
#include <Atrc/Material/BxDF/TorranceSparrow.h>
#include <Atrc/Material/Fresnel.h>

AGZ_NS_BEG(Atrc)

class Metal : public Material
{
    FresnelConductor fresnel_;
    BlinnPhongDistribution md_;
    TorranceSparrow torranceSparrow_;

public:

    Metal(const Spectrum &rc, const Spectrum &eta, const Spectrum &k, Real roughness);

    void Shade(const SurfacePoint &sp, ShadingPoint *dst, AGZ::ObjArena<> &arena) const override;
};

AGZ_NS_END(Atrc)
