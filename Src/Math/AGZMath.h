#pragma once

#include "../Common.h"

AGZ_NS_BEG(Atrc)

using namespace AGZ::Math;

using Real = double;

using RealT = FP<Real>;

using Vec2r = Vec2<Real>;
using Vec3r = Vec3<Real>;
using Vec4r = Vec4<Real>;
using Mat3r = Mat3<Real>;
using Mat4r = Mat4<Real>;

using Degr = Deg<Real>;
using Radr = Rad<Real>;

constexpr Real PIr = PI<Real>;
constexpr Real InvPIr = Real(1.0) / PIr;

inline Vec3r TransformBase(const Vec3r &dir,
                           const Vec3r &ex, const Vec3r &ey, const Vec3r &ez)
{
    return ex * dir.x + ey * dir.y + ez * dir.z;
}

inline Vec3r ReflectedDirection(const Vec3r &normal, const Vec3r &wi)
{
    return Real(2) * Dot(normal, wi) * normal - wi;
}

AGZ_NS_END(Atrc)
