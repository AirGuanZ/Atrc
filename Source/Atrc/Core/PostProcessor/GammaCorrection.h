#pragma once

#include <Atrc/Core/Core/PostProcessor.h>

namespace Atrc
{
    
class GammaCorrection : public PostProcessor
{
    Real gamma_;

public:

    explicit GammaCorrection(Real gamma) noexcept;

    void Process(Image *image) const noexcept override;
};

} // namespace Atrc
