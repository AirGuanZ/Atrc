#pragma once

#include <Atrc/Core/Core/PostProcessor.h>

namespace Atrc
{
    
class SaveAsHDR : public PostProcessor
{
    std::string filename_;

public:

    explicit SaveAsHDR(std::string filename) noexcept;

    void Process(Image *image) const noexcept override;
};

} // namespace Atrc
