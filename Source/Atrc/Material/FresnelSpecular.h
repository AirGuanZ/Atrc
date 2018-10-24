#pragma once

#include <Atrc/Core/Core.h>
#include <Atrc/Material/Fresnel.h>

AGZ_NS_BEG(Atrc)

class FresnelSpecular : public Material
{
    Spectrum rc_;
    RC<const FresnelDielectric> fresnel_;

public:

    void Shade(const SurfacePoint &sp, ShadingPoint *dst) const override;
};

AGZ_NS_END(Atrc)
