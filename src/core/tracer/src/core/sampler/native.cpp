#include <ctime>
#include <random>

#include <agz/tracer/core/sampler.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

class NativeSampler : public Sampler
{
    using rng_t = std::mt19937;
    using seed_t = rng_t::result_type;

    seed_t seed_ = 42;
    rng_t rng_;
    std::uniform_real_distribution<real> dis_;

    int spp_ = 1;
    int finished_spp_ = 0;

public:

    void initialize(int spp, int seed, bool use_time_seed)
    {
        if(use_time_seed)
            seed_ = static_cast<seed_t>(std::time(nullptr));
        else
            seed_ = static_cast<seed_t>(seed);
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
        auto *ret = arena.create<NativeSampler>();

        seed_t new_seed;
        {
            std::seed_seq seed_gen = { seed_, seed_t(seed) };
            seed_gen.generate(&new_seed, &new_seed + 1);
        }

        if(seed < 0)
            ret->rng_ = rng_;
        else
            ret->rng_ = rng_t(static_cast<seed_t>(new_seed));
        ret->seed_ = new_seed;

        ret->dis_ = dis_;
        ret->spp_ = spp_;

        return ret;
    }

    void start_pixel(int, int) override
    {
        finished_spp_ = 0;
    }

    bool next_sample() override
    {
        return ++finished_spp_ < spp_;
    }
};

std::shared_ptr<Sampler> create_native_sampler(
    int spp, int seed, bool use_time_seed)
{
    auto ret = std::make_shared<NativeSampler>();
    ret->initialize(spp, seed, use_time_seed);
    return ret;
}

AGZ_TRACER_END
