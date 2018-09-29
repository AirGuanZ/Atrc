#pragma once

#include "../Common.h"
#include "../Math/AGZMath.h"

AGZ_NS_BEG(Atrc::FresnelFormulae)

// Given eta_1, sin(theta_1) and eta_2,
// returns sin(theta_2) where theta_2 satisfys:
//     eta_1 * sin(theta_1) = eta_2 * sin(theta_2)
inline Real SnellLaw(Real eta1, Real sinTheta1, Real eta2)
{
    return eta1 * sinTheta1 / eta2;
}

AGZ_NS_END(Atrc::FresnelFormulae)
