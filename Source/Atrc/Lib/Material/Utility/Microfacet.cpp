#include <Atrc/Lib/Material/Utility/Microfacet.h>

namespace Atrc::Microfacet
{

namespace
{
    Real ComputeA(Real sinPhiH, Real cosPhiH, Real ax, Real ay)
    {
        return Sqr(cosPhiH / ax) + Sqr(sinPhiH / ay);
    }

} // namespace anonymous

Real GTR2(Real cosThetaH, Real alpha)
{
    return Sqr(alpha) / (PI * Sqr(1 + (Sqr(alpha) - 1) * Sqr(cosThetaH)));
}

Real SmithGTR2(Real tanTheta, Real alpha)
{
    if(!tanTheta)
        return 1;
    Real root = alpha * tanTheta;
    return 2 / (1 + Sqrt(1 + root * root));
}

Vec3 SampleGTR2(Real alpha, const Vec2 &sample)
{
    Real phi = 2 * PI * sample.x;
    Real cosTheta = Sqrt((1 - sample.y) / (1 + (Sqr(alpha) - 1) * sample.y));
    Real sinTheta = Cos2Sin(cosTheta);
    return Vec3(sinTheta * Cos(phi), sinTheta * Sin(phi), cosTheta).Normalize();
}

Real AnisotropicGTR2(Real sinPhiH, Real cosPhiH, Real sinThetaH, Real cosThetaH, Real ax, Real ay)
{
    Real A = ComputeA(sinPhiH, cosPhiH, ax, ay);
    Real RD = Sqr(sinThetaH) * A + Sqr(cosThetaH);
    return 1 / (PI * ax * ay * Sqr(RD));
}

Vec3 SampleAnisotropicGTR2(Real ax, Real ay, const Vec2 &sample)
{
    Real sinPhiH = ay * Sin(2 * PI * sample.x);
    Real cosPhiH = ax * Cos(2 * PI * sample.x);
    Real nor = 1 / Sqrt(Sqr(sinPhiH) + Sqr(cosPhiH));
    sinPhiH *= nor;
    cosPhiH *= nor;

    Real A = ComputeA(sinPhiH, cosPhiH, ax, ay);
    Real cosThetaH = Sqrt(A * (1 - sample.y) / ((1 - A) * sample.y + A));
    Real sinThetaH = Sqrt(Max<Real>(0, 1 - Sqr(cosThetaH)));

    return Vec3(sinThetaH * cosPhiH, sinThetaH * sinPhiH, cosThetaH).Normalize();

    /*Real A = Sqrt(sample.y / (1 - sample.y));
    Real xb = ax * Cos(2 * PI * sample.x);
    Real yb = ay * Sin(2 * PI * sample.x);
    Vec3 h(A * xb, A * yb, 1);
    return h.Normalize();*/
}

Real SmithAnisotropicGTR2(Real cosPhi, Real sinPhi, Real ax, Real ay, Real tanTheta)
{
    Real t = Sqr(ax * cosPhi) + Sqr(ay * sinPhi);
    Real sqr = 1 + t * Sqr(tanTheta);
    Real lambda = -Real(0.5) + Real(0.5) * Sqrt(sqr);
    return 1 / (1 + lambda);
}

} // namespace Atrc::GTR
