#pragma once

#include <Atrc/Core/Common.h>
#include <Atrc/Core/SceneParam.h>
#include <Atrc/Core/SurfacePoint.h>

AGZ_NS_BEG(Atrc)

class Material
{
public:

    virtual ~Material() = default;

    virtual Material *Clone(const SceneParamGroup &group, AGZ::ObjArena<> &arena) const;

    virtual void Shade(const SurfacePoint &sp, ShadingPoint *dst, AGZ::ObjArena<> &arena) const = 0;
};

AGZ_NS_END(Atrc)
