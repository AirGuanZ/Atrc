#include <Utils/Texture.h>
#include <Atrc/Lib/PostProcessor/SaveAsHDR.h>

namespace Atrc
{
    
SaveAsHDR::SaveAsHDR(Str8 filename) noexcept
    : filename_(std::move(filename))
{
    
}

void SaveAsHDR::Process(Image *image) const noexcept
{
    AGZ_ASSERT(image->IsAvailable());
    if constexpr(std::is_same_v<float, Real>)
        AGZ::TextureFile::WriteRGBToHDR(filename_, *image);
    else
    {
        AGZ::TextureFile::WriteRGBToHDR(
            filename_, image->Map([](const Spectrum &c)
        { return c.Map(AGZ::TypeOpr::StaticCaster<float, Real>); }));
    }
}

} // namespace Atrc
