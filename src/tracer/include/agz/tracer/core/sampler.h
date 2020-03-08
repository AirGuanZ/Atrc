#pragma once

#include <chrono>
#include <random>

#include <agz/tracer/common.h>

AGZ_TRACER_BEGIN

class Sampler : public misc::uncopyable_t
{
    using rng_t = std::minstd_rand;
    using seed_t = rng_t::result_type;

    seed_t seed_;
    rng_t rng_;
    std::uniform_real_distribution<real> dis_;

public:

    Sampler(int seed, bool use_time_seed);

    /**
     * @brief combine the new seed with internal seed to create a new sampler instance
     */
    Sampler *clone(int seed, Arena &arena) const;

    Sample1 sample1() noexcept;
    Sample2 sample2() noexcept;
    Sample3 sample3() noexcept;
    Sample4 sample4() noexcept;
    Sample5 sample5() noexcept;
};

inline Sampler::Sampler(int seed, bool use_time_seed)
    : rng_(seed)
{
    if(use_time_seed)
    {
        seed_ = static_cast<seed_t>(
            std::chrono::high_resolution_clock::now().time_since_epoch().count());
    }
    else
        seed_ = static_cast<seed_t>(seed);

    rng_ = rng_t(seed_);
}

inline Sampler *Sampler::clone(int seed, Arena &arena) const
{
    seed_t new_seed;
    {
        std::seed_seq seed_gen = { seed_, seed_t(seed) };
        seed_gen.generate(&new_seed, &new_seed + 1);
    }
    return arena.create<Sampler>(new_seed, false);
}

inline Sample1 Sampler::sample1() noexcept
{
    return { dis_(rng_) };
}

inline Sample2 Sampler::sample2() noexcept
{
    const real u = sample1().u;
    const real v = sample1().u;
    return { u, v };
}

inline Sample3 Sampler::sample3() noexcept
{
    const real u = sample1().u;
    const real v = sample1().u;
    const real w = sample1().u;
    return { u, v, w };
}

inline Sample4 Sampler::sample4() noexcept
{
    const real u = sample1().u;
    const real v = sample1().u;
    const real w = sample1().u;
    const real r = sample1().u;
    return { u, v, w, r };
}

inline Sample5 Sampler::sample5() noexcept
{
    const real u = sample1().u;
    const real v = sample1().u;
    const real w = sample1().u;
    const real r = sample1().u;
    const real s = sample1().u;
    return { u, v, w, r, s };
}

AGZ_TRACER_END
