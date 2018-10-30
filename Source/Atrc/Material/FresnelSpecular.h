#pragma once

#include <Atrc/Core/Core.h>
#include <Atrc/Material/Fresnel.h>

AGZ_NS_BEG(Atrc)

class FresnelSpecular : public Material
{
    Spectrum rc_;
    const Dielectric *fresnel_;

public:

    Material *Clone(const SceneParamGroup &group, AGZ::ObjArena<> &arena) const override;

    FresnelSpecular(const Spectrum &rc, const Dielectric *fresnel);

    void Shade(const SurfacePoint &sp, ShadingPoint *dst, AGZ::ObjArena<> &arena) const override;
};

AGZ_NS_END(Atrc)
