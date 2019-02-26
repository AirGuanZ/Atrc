#pragma once

#include <Atrc/Lib/Core/Common.h>

namespace Atrc::Microfacet
{

inline Real OneMinus5(Real x) { Real t = 1 - x, t2 = t * t; return t2 * t2 * t; }

Real GTR2(Real cosThetaH, Real alpha);

Real SmithGTR2(Real tanTheta, Real alpha);

Vec3 SampleGTR2(Real alpha, const Vec2 &sample);

Real AnisotropicGTR2(Real sinPhiH, Real cosPhiH, Real sinThetaH, Real cosThetaH, Real ax, Real ay);

Real SmithAnisotropicGTR2(Real cosPhi, Real sinPhi, Real ax, Real ay, Real tanTheta);

Vec3 SampleAnisotropicGTR2(Real ax, Real ay, const Vec2 &sample);

Real GTR1(Real sinThetaH, Real cosThetaH, Real alpha);

Vec3 SampleGTR1(Real alpha, const Vec2 &sample);

} // namespace Atrc::GTR
