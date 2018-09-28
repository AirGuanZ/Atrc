#pragma once

#include "../Math/AGZMath.h"

AGZ_NS_BEG(Atrc)

using Sample2D = Vec2r;

class UniformlySampleOnHemisphere
{
public:

    static Vec3r Sample(const Sample2D &stdSample);

    static Real PDF(const Vec3r &sample);
};

class UniformlySampleOnSphere
{
public:

    static Vec3r Sample(const Sample2D &stdSample);

    static Real PDF(const Vec3r &sample);
};

class ZWeightedSampleOnHemisphere
{
public:

    static Vec3r Sample(const Sample2D &stdSample);

    static Real PDF(const Vec3r &sample);
};

class UniformltSampleOnCircle
{
public:

    static Vec2r Sample(Real stdSample);

    static Real PDF(const Vec2r &sample);
};

AGZ_NS_END(Atrc)
