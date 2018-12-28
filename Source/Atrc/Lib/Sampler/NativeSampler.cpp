#include <Atrc/Lib/Sampler/NativeSampler.h>

namespace Atrc
{
    
NativeSampler::NativeSampler(int seed, int spp)
        noexcept(noexcept(std::minstd_rand0(std::declval<int>())))
    : dis_(0, 1), rng_(seed), initSeed_(seed), spp_(spp), remainSpp_(0)
{
    AGZ_ASSERT(spp >= 1);
}

std::unique_ptr<Sampler> NativeSampler::Clone(int seed) const
{
    return std::make_unique<NativeSampler>(initSeed_ * seed, spp_);
}

void NativeSampler::StartPixel(const Vec2i &pixel)
{
    remainSpp_ = spp_ - 1;
    pixel_ = pixel;
}

bool NativeSampler::NextSample()
{
    return remainSpp_-- > 0;
}

Camera::CameraSample NativeSampler::GetCameraSample()
{
    Camera::CameraSample ret;
    ret.film = pixel_.Map(AGZ::TypeOpr::StaticCaster<Real, int32_t>) + GetReal2();
    ret.lens = GetReal2();
    return ret;
}

Real NativeSampler::GetReal()
{
    return dis_(rng_);
}

}
