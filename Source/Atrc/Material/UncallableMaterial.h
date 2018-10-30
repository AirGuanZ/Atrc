#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class UncallableMaterial : public Material
{
public:

    void Shade(const SurfacePoint &sp, ShadingPoint *dst, AGZ::ObjArena<> &arena) const override
    {
        throw UnreachableException("UncallableMaterial");
    }
};

AGZ_NS_END(Atrc)
