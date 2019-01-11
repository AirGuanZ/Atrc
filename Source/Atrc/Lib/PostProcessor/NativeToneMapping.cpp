#include <AGZUtils/Utils/Texture.h>
#include <Atrc/Lib/PostProcessor/NativeToneMapping.h>

namespace Atrc
{

NativeToneMapping::NativeToneMapping(const Spectrum &LWhite) noexcept
    : LWhite2_(LWhite * LWhite)
{
    AGZ_ASSERT(LWhite2_.x && LWhite2_.y && LWhite2_.z);
}

void NativeToneMapping::Process(Image *image) const noexcept
{
    for(uint32_t y = 0; y < image->GetHeight(); ++y)
    {
        for(uint32_t x = 0; x < image->GetWidth(); ++x)
        {
            auto L = image->At(x, y);
            image->At(x, y) = L * (1 + L / LWhite2_) / (1 + L);
        }
    }
}

} // namespace Atrc
