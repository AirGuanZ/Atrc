#pragma once

#include <Atrc/Lib/Core/PostProcessor.h>

namespace Atrc
{
    
class NativeToneMapping : public PostProcessor
{
    Spectrum LWhite2_;

public:

    explicit NativeToneMapping(const Spectrum &LWhite) noexcept;

    void Process(Image *image) const noexcept override;
};

} // namespace Atrc
