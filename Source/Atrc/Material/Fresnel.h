#pragma once

#include <Atrc/Core/Core.h>

// 这部分实现参考了PBRT-3rd

AGZ_NS_BEG(Atrc)

// 绝缘体Fresnel反射比
float FresnelDielectric(float etaI, float etaT, float cosThetaT);

// 导体Fresnel反射比
Spectrum FresnelConductor(const Spectrum &etaI, const Spectrum &etaT, const Spectrum &k, float cosThetaI);

AGZ_NS_END(Atrc)
