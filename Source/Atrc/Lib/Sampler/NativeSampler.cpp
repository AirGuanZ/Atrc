#include <Atrc/Lib/Sampler/NativeSampler.h>

namespace Atrc
{
    
NativeSampler::NativeSampler(int seed, int spp)
        noexcept(noexcept(std::default_random_engine(std::declval<int>())))
    : dis_(0, 1), rng_(seed), spp_(spp), remainSpp_(0)
{
    AGZ_ASSERT(spp >= 1);
}

std::unique_ptr<Sampler> NativeSampler::Clone(int seed) const
{
    return std::make_unique<NativeSampler>(seed, spp_);
}

void NativeSampler::StartPixel(const Vec2i &pixel)
{
    remainSpp_ = spp_;
    pixel_ = pixel;
}

bool NativeSampler::NextSample()
{
    return remainSpp_-- > 0;
}

Camera::CameraSample NativeSampler::GetCameraSample()
{
    Camera::CameraSample ret;
    ret.film = pixel_.Map(AGZ::TypeOpr::StaticCaster<Real, int32_t>) + GetSample2D();
    ret.lens = GetSample2D();
    return ret;
}

Real NativeSampler::GetSample()
{
    return dis_(rng_);
}

Vec2 NativeSampler::GetSample2D()
{
    Real x = dis_(rng_), y = dis_(rng_);
    return { x, y };
}

}
