#include <AGZUtils/Utils/Texture.h>

#include <Atrc/Lib/PostProcessor/GammaCorrection.h>

namespace Atrc
{
    
GammaCorrection::GammaCorrection(Real gamma) noexcept
    : gamma_(gamma)
{
    
}

void GammaCorrection::Process(Image *image) const noexcept
{
    AGZ_ASSERT(image);
    for(uint32_t y = 0; y < image->GetHeight(); ++y)
    {
        for(uint32_t x = 0; x < image->GetWidth(); ++x)
            image->At(x, y) = image->At(x, y).Map([=](Real c) { return Pow(c, gamma_); });
    }
}

} // namespace Atrc
