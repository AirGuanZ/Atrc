#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class UncallableMaterial : public Material
{
public:

    Material *Clone(const SceneParamGroup &group, AGZ::ObjArena<> &arena) const override
    {
        return arena.Create<UncallableMaterial>();
    }

    void Shade(const SurfacePoint &sp, ShadingPoint *dst, AGZ::ObjArena<> &arena) const override
    {
        throw UnreachableException("UncallableMaterial");
    }
};

AGZ_NS_END(Atrc)
