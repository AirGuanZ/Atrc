#pragma once

#include <Atrc/Core/Core/Common.h>

// SHOrder \in [0, 5)
// N > 0
void Light2SH(const Atrc::Light *light, int SHOrder, int N, Atrc::Spectrum *coefs);
