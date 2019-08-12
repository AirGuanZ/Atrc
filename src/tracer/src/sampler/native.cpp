#include <ctime>
#include <random>

#include <agz/tracer/core/sampler.h>

AGZ_TRACER_BEGIN

class NativeSampler : public Sampler
{
    using rng_t = std::mt19937;
    using seed_t = rng_t::result_type;

    rng_t rng_;
    std::uniform_real_distribution<real> dis_;

    int spp_ = 1;
    int finished_spp_ = 0;

public:

    using Sampler::Sampler;

    static std::string description()
    {
        return R"___(
native [Sampler]
    seed [int] (optional) rng seed (defautly generated with std::time)
    spp  [int] samples per pixel
    min_spp [int] minimum spp before enabling prestop
    stop_noinct_ratio [real] prestop threshold
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &) override
    {
        AGZ_HIERARCHY_TRY

        seed_t seed;
        if(auto node = params.find_child("seed"))
            seed = static_cast<seed_t>(node->as_value().as_int());
        else
            seed = static_cast<seed_t>(std::time(nullptr));
        rng_ = rng_t(seed);

        dis_ = std::uniform_real_distribution<real>();

        spp_ = params.child_int("spp");
        if(spp_ < 1)
            throw ObjectConstructionException("invalid spp value: " + std::to_string(spp_));

        AGZ_HIERARCHY_WRAP("in initializing native sampler")
    }

    Sample1 sample1() noexcept override
    {
        return { dis_(rng_) };
    }

    Sampler *clone(int seed, Arena &arena) const override
    {
        auto *ret = arena.create<NativeSampler>();

        if(seed < 0)
            ret->rng_ = rng_;
        else
            ret->rng_ = rng_t(static_cast<seed_t>(seed));

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

    void report(const Spectrum &rad) override
    {

    }
};

AGZT_IMPLEMENTATION(Sampler, NativeSampler, "native")

AGZ_TRACER_END
