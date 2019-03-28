#include <AGZUtils/Utils/Texture.h>

#include <Atrc/Core/PostProcessor/SaveAsHDR.h>

namespace Atrc
{
    
SaveAsHDR::SaveAsHDR(std::string filename) noexcept
    : filename_(std::move(filename))
{
    
}

void SaveAsHDR::Process(Image *image) const noexcept
{
    AGZ_ASSERT(image->IsAvailable());
#ifdef AGZ_CC_GCC
    AGZ::TextureFile::WriteRGBToHDR(
        filename_, image->Map([](const Spectrum &c)->AGZ::Math::Vec3<float>
    { return c.Map([](Real v)->float { return float(v); }); }));
#else
    AGZ::TextureFile::WriteRGBToHDR(
        filename_, image->Map([](const Spectrum &c)
    { return c.Map(AGZ::TypeOpr::StaticCaster<float, Real>); }));
#endif
}

} // namespace Atrc
