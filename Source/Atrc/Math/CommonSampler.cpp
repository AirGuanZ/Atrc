#include <Atrc/Math/CommonSampler.h>

AGZ_NS_BEG(Atrc::CommonSampler)

namespace
{
    thread_local Random::SharedRandomEngine_t<std::minstd_rand> RNG;
}

Uniform_InUnitSphere::Result Uniform_InUnitSphere::Sample()
{
    for(;;)
    {
        Vec3r sample(
            2.0 * Uniform(0.0, 1.0, RNG) - 1.0,
            2.0 * Uniform(0.0, 1.0, RNG) - 1.0,
            2.0 * Uniform(0.0, 1.0, RNG) - 1.0);
        if(sample.LengthSquare() <= 1.0)
            return { sample, 4.0 / 3.0 * PI<Real> };
    }
}

Real Uniform_InUnitSphere::PDF(const Vec3r &sample)
{
    return 1 / (4.0 / 3.0 * PI<Real>);
}

namespace
{
    Vec2r Concentric_UnitDisk()
    {
        Vec2r uOffset = 2 * Vec2r(Uniform(0.0, 1.0, RNG), Uniform(0.0, 1.0, RNG)) - Vec2r(1.0);
        if(uOffset.x == 0 && uOffset.y == 0)
            return Vec2r(0.0);
        Real theta, r;
        if(Abs(uOffset.x) > Abs(uOffset.y)) {
            r = uOffset.x;
            theta = 0.25 * PI<Real> * (uOffset.y / uOffset.x);
        }
        else {
            r = uOffset.y;
            theta = 0.5 * PI<Real> - 0.25 * PI<Real> * (uOffset.x / uOffset.y);
        }
        return r * Vec2r(Cos(theta), Sin(theta));
    }
}

ZWeighted_OnUnitHemisphere::Result ZWeighted_OnUnitHemisphere::Sample()
{
    auto d = Concentric_UnitDisk();
    Real z = std::sqrt(std::max(0.0, 1.0 - d.LengthSquare()));
    return { Vec3r(d, z), z / PI<Real> };
}

Real ZWeighted_OnUnitHemisphere::PDF(const Vec3r &sample)
{
    return sample.z / PI<Real>;
}

AGZ_NS_END(Atrc::CommonSampler)
