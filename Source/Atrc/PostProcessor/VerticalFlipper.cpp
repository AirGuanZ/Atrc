#include <Atrc/PostProcessor/VerticalFlipper.h>

AGZ_NS_BEG(Atrc)

void VerticalFlipper::Process(RenderTarget &img) const
{
    RenderTarget t(img.GetWidth(), img.GetHeight());
    for(uint32_t y = 0; y < t.GetHeight(); ++y)
    {
        for(uint32_t x = 0; x < t.GetWidth(); ++x)
            t(x, y) = img(x, t.GetHeight() - y - 1);
    }
    img = std::move(t);
}

AGZ_NS_END(Atrc)
