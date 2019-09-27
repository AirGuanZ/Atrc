#include <ctime>
#include <random>

#include <agz/tracer/core/sampler.h>

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

    explicit NativeSampler(CustomedFlag customed_flag = DEFAULT_CUSTOMED_FLAG)
    {
        object_customed_flag_ = customed_flag;
    }

    static std::string description()
    {
        return R"___(
native [Sampler]
    seed [int] (optional) rng seed (defautly generated with std::time)
    spp  [int] samples per pixel
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &) override
    {
        AGZ_HIERARCHY_TRY

        init_customed_flag(params);

        seed_t seed;
        if(auto node = params.find_child("seed"))
            seed = static_cast<seed_t>(node->as_value().as_int());
        else
            seed = static_cast<seed_t>(std::time(nullptr));
        rng_ = rng_t(seed);
        seed_ = seed;

        dis_ = std::uniform_real_distribution<real>();

        spp_ = params.child_int("spp");
        if(spp_ < 1)
            throw ObjectConstructionException("invalid spp value: " + std::to_string(spp_));

        AGZ_HIERARCHY_WRAP("in initializing native sampler")
    }

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

Sampler *create_native_sampler(
    int spp, int seed, bool use_time_seed,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena)
{
    auto ret = arena.create<NativeSampler>(customed_flag);
    ret->initialize(spp, seed, use_time_seed);
    return ret;
}

AGZT_IMPLEMENTATION(Sampler, NativeSampler, "native")

AGZ_TRACER_END
