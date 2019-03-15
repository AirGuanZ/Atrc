#pragma once

#include <Atrc/Lib/Core/Common.h>

Atrc::Image ReconstructImage(int SHOrder, const Atrc::Image *sceneImages, const Atrc::Spectrum *lightCoefs);
