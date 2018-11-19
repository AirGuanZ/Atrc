#include <Atrc/PostProcessor/GammaCorrector.h>

AGZ_NS_BEG(Atrc)

GammaCorrector::GammaCorrector(float gamma)
    : gamma_(gamma)
{
    AGZ_ASSERT(gamma > 0.0);
}

void GammaCorrector::Process(RenderTarget &img) const
{
    img = img.Map([=](const Spectrum &s)
    {
        return s.Map([=](float c) { return Pow(c, gamma_); });
    });
}

AGZ_NS_END(Atrc)
