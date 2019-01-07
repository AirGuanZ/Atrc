#pragma once

#include <Atrc/Lib/Core/PostProcessor.h>

namespace Atrc
{
    
class SaveAsHDR : public PostProcessor
{
    Str8 filename_;

public:

    explicit SaveAsHDR(Str8 filename) noexcept;

    void Process(Image *image) const noexcept override;
};

} // namespace Atrc
