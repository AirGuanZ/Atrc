#pragma once

#include <Atrc/Lib/Core/Common.h>

namespace Atrc::SH2D
{

class LightProjector
{
    int SHOrder_;

public:

    explicit LightProjector(int SHOrder) noexcept;

    void Project(const Light *light, int sampleCount, Spectrum *coefs) const;
};

} // namespace Atrc::SH2D
