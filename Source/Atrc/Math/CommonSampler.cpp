#include <Atrc/Math/CommonSampler.h>

AGZ_NS_BEG(Atrc::CommonSampler)

Uniform_InUnitSphere::Result Uniform_InUnitSphere::Sample()
{
    for(;;)
    {
        Vec3r sample(
            2.0 * Rand() - 1.0,
            2.0 * Rand() - 1.0,
            2.0 * Rand() - 1.0);
        if(sample.LengthSquare() <= 1.0)
            return { sample, 4.0 / 3.0 * PI<Real> };
    }
}

Real Uniform_InUnitSphere::PDF(const Vec3r &sample)
{
    return 4.0 / 3.0 * PI<Real>;
}

ZWeighted_OnUnitHemisphere::Result ZWeighted_OnUnitHemisphere::Sample()
{
    // IMPROVE
    Real phi = 2.0 * PI<Real> * Rand();

    Real V = Rand();
    Real theta = Arccos(1.0 - V);

    Real sinTheta = Sin(theta);
    Real cosTheta = Sqrt(std::max(0.0, 1.0 - sinTheta * sinTheta));

    return { { cosTheta * Cos(phi), cosTheta * Sin(phi), sinTheta }, sinTheta };
}

Real ZWeighted_OnUnitHemisphere::PDF(const Vec3r &sample)
{
    return sample.z;
}

AGZ_NS_END(Atrc::CommonSampler)
