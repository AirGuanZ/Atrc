#pragma once

#include <random>
#include <type_traits>

#include <Atrc/Lib/Core/Sampler.h>

namespace Atrc
{

class NativeSampler : public Sampler
{
    using StdRNG = std::default_random_engine;

    std::uniform_real_distribution<Real> dis_;
    StdRNG rng_;

    int initSeed_;

    int spp_;
    int remainSpp_;

    Vec2i pixel_;

public:

    NativeSampler(int seed, int spp)
        noexcept(noexcept(StdRNG(std::declval<int>())));
    
    std::unique_ptr<Sampler> Clone(int seed) const override;

    void StartPixel(const Vec2i &pixel) override;

    bool NextSample() override;

    Camera::CameraSample GetCameraSample() override;

    Real GetReal() override;
};

} // namespace Atrc
