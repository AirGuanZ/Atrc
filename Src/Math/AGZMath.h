#pragma once

#include "../Common.h"

AGZ_NS_BEG(Atrc)

using namespace AGZ::Math;

using Real = double;

using RealT = FP<Real>;

using Vec2r = Vec2<Real>;
using Vec3r = Vec3<Real>;
using Vec4r = Vec4<Real>;
using Mat4r = Mat4<Real>;

using Degr = Deg<Real>;
using Radr = Rad<Real>;

constexpr Real PIr = PI<Real>;
constexpr Real InvPIr = Real(1.0) / PIr;

AGZ_NS_END(Atrc)
