#include <Atrc/Math/CommonSampler.h>

AGZ_NS_BEG(Atrc::CommonSampler)

Uniform_InUnitSphere::Result Uniform_InUnitSphere::Sample()
{
    for(;;)
    {
        Vec3r sample(
            Real(2) * Rand() - Real(1),
            Real(2) * Rand() - Real(1),
            Real(2) * Rand() - Real(1));
        if(sample.LengthSquare() <= Real(1))
            return { sample, Real(4) / Real(3) * PI<Real> };
    }
}

Real Uniform_InUnitSphere::PDF(const Vec3r &sample)
{
    return Real(4) / Real(3) * PI<Real>;
}

ZWeighted_OnUnitHemisphere::Result ZWeighted_OnUnitHemisphere::Sample()
{
    // IMPROVE
    Real phi = Real(2) * PI<Real> * Rand();

    Real V = Rand();
    Real theta = Arccos(Real(1) - V);

    Real sinTheta = Sin(theta);
    Real cosTheta = Sqrt(std::max(Real(0), Real(1) - sinTheta * sinTheta));

    return { { cosTheta * Cos(phi), cosTheta * Sin(phi), sinTheta }, sinTheta };
}

Real ZWeighted_OnUnitHemisphere::PDF(const Vec3r &sample)
{
    return sample.z;
}

AGZ_NS_END(Atrc::CommonSampler)
