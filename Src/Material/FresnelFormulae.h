#pragma once

#include "../Common.h"
#include "../Math/AGZMath.h"

AGZ_NS_BEG(Atrc::FresnelFormulae)

inline Vec3r ReflectedDirection(const Vec3r &normal, const Vec3r &wi)
{
    return Real(2) * Dot(normal, wi) * normal - wi;
}

// Given eta_1, sin(theta_1) and eta_2,
// returns sin(theta_2) where theta_2 satisfys:
//     eta_1 * sin(theta_1) = eta_2 * sin(theta_2)
inline Real SnellLaw(Real eta_1, Real sinTheta_1, Real eta_2)
{
    return eta_1 * sinTheta_1 / eta_2;
}

Option<Vec3r> TransmittedDirection(
    const Vec3r &normal, const Vec3r &wi, Real eta_i, Real eta_t);

// Fresnel reflectance formulae for dielectrics
Real DielectricsFresnelReflection(
    Real eta_i, Real eta_t, Real cosTheta_i, Real cosTheta_t);

inline Real DielectricsFresnelTransmission(
    Real eta_i, Real eta_t, Real cosTheta_i, Real cosTheta_t)
{
    return Real(1) - DielectricsFresnelReflection(
                        eta_i, eta_t, cosTheta_i, cosTheta_t);
}

AGZ_NS_END(Atrc::FresnelFormulae)
