#include <ctime>
#include <random>

#include <agz/tracer/core/sampler.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

class NativeSampler : public Sampler
{
    using rng_t = std::minstd_rand;
    using seed_t = rng_t::result_type;

    seed_t seed_ = 42;
    rng_t rng_;
    std::uniform_real_distribution<real> dis_;

    int spp_ = 1;
    int finished_spp_ = 0;

public:

    NativeSampler(int spp, int seed, bool use_time_seed)
    {
        if(use_time_seed)
            seed_ = static_cast<seed_t>(std::time(nullptr));
        else
            seed_ = static_cast<seed_t>(seed);

        rng_ = rng_t(seed_);

        spp_ = spp;
        if(spp_ < 1)
            throw ObjectConstructionException("invalid spp value: " + std::to_string(spp_));
    }

    Sample1 sample1() noexcept override
    {
        return { dis_(rng_) };
    }

    Sampler *clone(int seed, Arena &arena) const override
    {
        seed_t new_seed;
        {
            std::seed_seq seed_gen = { seed_, seed_t(seed) };
            seed_gen.generate(&new_seed, &new_seed + 1);
        }
        return arena.create<NativeSampler>(spp_, new_seed, false);
    }

    void start_pixel(int, int) override
    {
        finished_spp_ = 0;
    }

    bool next_sample() override
    {
        return ++finished_spp_ < spp_;
    }

    int get_spp() const noexcept override
    {
        return spp_;
    }
};

std::shared_ptr<Sampler> create_native_sampler(
    int spp, int seed, bool use_time_seed)
{
    return std::make_shared<NativeSampler>(spp, seed, use_time_seed);
}

AGZ_TRACER_END
