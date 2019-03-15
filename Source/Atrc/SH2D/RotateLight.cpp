#include <Atrc/SH2D/RotateLight.h>

using namespace Atrc;

void RotateLightCoefs(const Atrc::Mat3 &rotateMatrix, int SHOrder, Spectrum *coefs)
{
    int SHC = SHOrder * SHOrder;
    std::vector<std::vector<Real>> channels(
        SPECTRUM_CHANNEL_COUNT, std::vector<Real>(SHC));

    for(int c = 0; c < SPECTRUM_CHANNEL_COUNT; ++c)
    {
        for(int s = 0; s < SHC; ++s)
            channels[c][s] = coefs[s][c];
    }

    auto rot = [&](int offset, void(*f)(const Mat3&, Real*))
    {
        for(int c = 0; c < SPECTRUM_CHANNEL_COUNT; ++c)
            f(rotateMatrix, &channels[c][offset]);
    };

    if(SHOrder >= 0) rot(0,  &AGZ::Math::SH::RotateSH_L0<Real>);
    if(SHOrder >= 1) rot(1,  &AGZ::Math::SH::RotateSH_L1<Real>);
    if(SHOrder >= 2) rot(4,  &AGZ::Math::SH::RotateSH_L2<Real>);
    if(SHOrder >= 3) rot(9,  &AGZ::Math::SH::RotateSH_L3<Real>);
    if(SHOrder >= 4) rot(16, &AGZ::Math::SH::RotateSH_L4<Real>);

    for(int c = 0; c < SPECTRUM_CHANNEL_COUNT; ++c)
    {
        for(int s = 0; s < SHC; ++s)
            coefs[s][c] = channels[c][s];
    }
}
