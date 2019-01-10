#include <AGZUtils/Utils/Texture.h>

#include <Atrc/Lib/PostProcessor/ACESFilm.h>

namespace Atrc
{
    
void ACESFilm::Process(Image *img) const noexcept
{
    *img = img->Map([=](const Spectrum &s)
    {
        return s.Map([=](float x)
        {
            constexpr Real a = Real(2.51);
            constexpr Real b = Real(0.03);
            constexpr Real c = Real(2.43);
            constexpr Real d = Real(0.59);
            constexpr Real e = Real(0.14);
            return (x * (a * x + b)) / (x * (c * x + d) + e);
        });
    });
}

} // namespace Atrc
