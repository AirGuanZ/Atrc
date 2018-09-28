#include "Sample.h"

AGZ_NS_BEG(Atrc)

Vec3r UniformlySampleOnHemisphere::Sample(const Sample2D &stdSample)
{
    Real phi = Real(2) * PIr * stdSample.u;
    Real sin = stdSample.v;
    Real cos = Sqrt(std::max(Real(0), Real(1) - sin * sin));
    return {
        cos * Cos(phi),
        cos * Sin(phi),
        sin
    };
}

Real UniformlySampleOnHemisphere::PDF(const Vec3r &sample)
{
    return Real(0.5) * InvPIr;
}

Vec3r UniformlySampleOnSphere::Sample(const Sample2D &stdSample)
{
    Real phi = Real(2) * PIr * stdSample.u;
    Real sin = Real(2) * stdSample.v - Real(1);
    Real cos = Sqrt(std::max(Real(0), Real(1) - sin * sin));
    return {
        cos * Cos(phi),
        cos * Sin(phi),
        sin
    };
}

Real UniformlySampleOnSphere::PDF(const Vec3r &sample)
{
    return Real(0.25) * InvPIr;
}

Vec3r ZWeightedSampleOnHemisphere::Sample(const Sample2D &stdSample)
{
    Real phi = Real(2) * PIr * stdSample.u;
    Real theta = Real(0.5) * PIr * stdSample.v;
    Real z = Real(1) - Cos(theta);
    Real r = Sqrt(std::max(Real(0), Real(1) - z * z));
    return {
        r * Cos(phi),
        r * Sin(phi),
        z
    };
}

Real ZWeightedSampleOnHemisphere::PDF(const Vec3r &sample)
{
    Real cos = Real(1) - sample.z * sample.z;
    return Sqrt(std::max(Real(0), Real(1) - cos * cos));
}

Vec2r UniformltSampleOnCircle::Sample(Real stdSample)
{
    Real phi = Real(2) * PIr * stdSample;
    return { Cos(phi), Sin(phi) };
}

Real UniformltSampleOnCircle::PDF(const Vec2r &sample)
{
    return Real(0.5) * InvPIr;
}

AGZ_NS_END(Atrc)
