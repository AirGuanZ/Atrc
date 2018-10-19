#pragma once

#include <Atrc/Core/Common.h>
#include <Atrc/Core/SurfacePoint.h>

AGZ_NS_BEG(Atrc)

class Material
{
public:

    virtual ~Material() = default;

    // 给定sp求其shdLocal，除非实现了bump mapping等技术，否则一般就照搬geoLocal
    virtual void ComputeShadingLocal(SurfacePoint *sp) const;

    virtual void Shade(const SurfacePoint &sp, ShadingPoint *dst) const = 0;
};

AGZ_NS_END(Atrc)
