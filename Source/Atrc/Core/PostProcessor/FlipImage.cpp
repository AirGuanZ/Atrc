#include <AGZUtils/Utils/Texture.h>

#include <Atrc/Core/PostProcessor/FlipImage.h>

namespace Atrc
{
    
void FlipImage::Process(Image *image) const noexcept
{
    AGZ_ASSERT(image && image->IsAvailable());

    uint32_t w = image->GetWidth(), h = image->GetHeight();
    uint32_t xMid = w / 2, yMid = h / 2;

    // 水平翻转

    for(uint32_t y = 0; y < h; ++y)
    {
        for(uint32_t x = 0; x < xMid; ++x)
            std::swap(image->At(x, y), image->At(w - 1 - x, y));
    }

    // 垂直翻转

    for(uint32_t y = 0; y < yMid; ++y)
    {
        for(uint32_t x = 0; x < w; ++x)
            std::swap(image->At(x, y), image->At(x, h - 1 - y));
    }
}

} // namespace Atrc
