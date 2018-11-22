#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class UncallableMaterial : public Material
{
public:

    void Shade([[maybe_unused]] const SurfacePoint &sp, [[maybe_unused]] ShadingPoint *dst, [[maybe_unused]] AGZ::ObjArena<> &arena) const override
    {
        throw UnreachableException("UncallableMaterial");
    }
};

inline UncallableMaterial STATIC_UNCALLABLE_MATERIAL;

AGZ_NS_END(Atrc)
