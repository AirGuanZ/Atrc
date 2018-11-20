#include <Atrc/PostProcessor/ACESFilm.h>

AGZ_NS_BEG(Atrc)

void ACESFilm::Process(RenderTarget &img) const
{
    img = img.Map([=](const Spectrum &s)
    {
        return s.Map([=](float x)
        {

            constexpr float a = 2.51f;
            constexpr float b = 0.03f;
            constexpr float c = 2.43f;
            constexpr float d = 0.59f;
            constexpr float e = 0.14f;
            return (x * (a * x + b)) / (x * (c * x + d) + e);
        });
    });
}

AGZ_NS_END(Atrc)
