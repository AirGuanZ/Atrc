#pragma once

#include <chrono>
#include <random>

#include <agz/tracer/common.h>

AGZ_TRACER_BEGIN

class Sampler
{
public:

    virtual ~Sampler() = default;

    virtual Sample1 sample1() = 0;
    virtual Sample2 sample2() = 0;
    virtual Sample3 sample3() = 0;
    virtual Sample4 sample4() = 0;
    virtual Sample5 sample5() = 0;
};

class NativeSampler : public Sampler
{
public:

    using rng_t = std::minstd_rand;
    using seed_t = rng_t::result_type;

    NativeSampler(int seed, bool use_time_seed);

    /**
     * @brief combine new seed with internal seed to create a sampler instance
     */
    NativeSampler *clone(int seed, Arena &arena) const;

    Sample1 sample1() override;
    Sample2 sample2() override;
    Sample3 sample3() override;
    Sample4 sample4() override;
    Sample5 sample5() override;

    rng_t &rng() noexcept { return rng_; }

private:

    seed_t seed_;
    rng_t rng_;
    std::uniform_real_distribution<real> dis_;
};

inline NativeSampler::NativeSampler(int seed, bool use_time_seed)
    : rng_(seed)
{
    if(use_time_seed)
    {
        seed_ = static_cast<seed_t>(
            std::chrono::high_resolution_clock::now()
            .time_since_epoch().count());
    }
    else
        seed_ = static_cast<seed_t>(seed);

    rng_ = rng_t(seed_);
}

inline NativeSampler *NativeSampler::clone(int seed, Arena &arena) const
{
    seed_t new_seed;
    {
        std::seed_seq seed_gen = { seed_, seed_t(seed) };
        seed_gen.generate(&new_seed, &new_seed + 1);
    }
    return arena.create<NativeSampler >(new_seed, false);
}

inline Sample1 NativeSampler::sample1()
{
    return { dis_(rng_) };
}

inline Sample2 NativeSampler::sample2()
{
    const real u = sample1().u;
    const real v = sample1().u;
    return { u, v };
}

inline Sample3 NativeSampler::sample3()
{
    const real u = sample1().u;
    const real v = sample1().u;
    const real w = sample1().u;
    return { u, v, w };
}

inline Sample4 NativeSampler::sample4()
{
    const real u = sample1().u;
    const real v = sample1().u;
    const real w = sample1().u;
    const real r = sample1().u;
    return { u, v, w, r };
}

inline Sample5 NativeSampler::sample5()
{
    const real u = sample1().u;
    const real v = sample1().u;
    const real w = sample1().u;
    const real r = sample1().u;
    const real s = sample1().u;
    return { u, v, w, r, s };
}

AGZ_TRACER_END
