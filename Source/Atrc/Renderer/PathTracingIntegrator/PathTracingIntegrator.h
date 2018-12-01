#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class PathTracingIntegrator
{
public:

    virtual ~PathTracingIntegrator() = default;

    virtual Spectrum Eval(const Scene &scene, const Ray &r, AGZ::ObjArena<> &arena) const = 0;
};

AGZ_NS_END(Atrc)
