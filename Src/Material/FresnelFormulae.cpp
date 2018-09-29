#include "FresnelFormulae.h"

AGZ_NS_BEG(Atrc::FresnelFormulae)

Option<Vec3r> TransmittedDirection(
    const Vec3r &normal, const Vec3r &wi, Real eta_i, Real eta_t)
{
    if(ApproxEq(normal, wi, Real(1e-4)))
        return -wi;

    Real cosTheta_i = Dot(normal, wi);
    Real sinTheta_i = Sqrt(std::max(Real(0), 1 - cosTheta_i * cosTheta_i));

    // Full reflection
    if(sinTheta_i >= eta_t / eta_i)
        return None;

    Real sinTheta_t = SnellLaw(eta_i, sinTheta_i, eta_t);
    Real cosTheta_t = Sqrt(std::max(Real(0), 1 - sinTheta_t * sinTheta_t));

    Vec3r a = (sinTheta_t / sinTheta_i) * (cosTheta_i * normal - wi);
    return a - cosTheta_t * normal;
}

Real DielectricsFresnelReflection(
    Real eta_i, Real eta_t, Real cosTheta_i, Real cosTheta_t)
{
    Real rPara = (eta_t * cosTheta_i - eta_i * cosTheta_t) /
                 (eta_t * cosTheta_i + eta_i*  cosTheta_t);
    Real rPerp = (eta_i * cosTheta_i - eta_t * cosTheta_t) /
                 (eta_i * cosTheta_i + eta_t * cosTheta_t);
    return Real(0.5) * (rPara * rPara + rPerp * rPerp);
}

AGZ_NS_END(Atrc::FresnelFormulae)
