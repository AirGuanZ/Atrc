#pragma once

#include <random>
#include <type_traits>

#include <Atrc/Lib/Core/Sampler.h>

namespace Atrc
{

class NativeSampler : public Sampler
{
    std::uniform_real_distribution<Real> dis_;
    std::default_random_engine rng_;

    int initSeed_;

    int spp_;
    int remainSpp_;

    Vec2i pixel_;

public:

    NativeSampler(int seed, int spp)
        noexcept(noexcept(std::minstd_rand0(std::declval<int>())));
    
    std::unique_ptr<Sampler> Clone(int seed) const override;

    void StartPixel(const Vec2i &pixel) override;

    bool NextSample() override;

    Camera::CameraSample GetCameraSample() override;

    Real GetReal() override;

    Vec2 GetReal2() override;

    Vec3 GetReal3() override;
};

} // namespace Atrc
