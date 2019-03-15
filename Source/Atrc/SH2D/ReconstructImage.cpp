#include <Atrc/Lib/Core/TFilm.h>
#include <Atrc/SH2D/ReconstructImage.h>

using namespace Atrc;

Atrc::Image ReconstructImage(int SHOrder, const Image *sceneCoefs, const Spectrum *lightCoefs)
{
    int SHC = SHOrder * SHOrder;
    auto resolution = sceneCoefs[0].GetSize();
    Image ret(resolution[0], resolution[1]);

    for(uint32_t y = 0; y < ret.GetHeight(); ++y)
    {
        for(uint32_t x = 0; x < ret.GetWidth(); ++x)
        {
            Spectrum pixel;
            for(int i = 0; i < SHC; ++i)
                pixel += sceneCoefs[i](x, y) * lightCoefs[i];
            ret(x, y) = pixel;
        }
    }

    return ret;
}
