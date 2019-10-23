#include <random>

#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/scene.h>

#include "./path_traced_bssrdf.h"

AGZ_TRACER_BEGIN

namespace
{
    class LocalSampler
    {
    public:

        using RNG = std::mt19937;
        using Seed = RNG::result_type;

        explicit LocalSampler(Seed init_seed) noexcept
            : rng_(init_seed)
        {
            
        }

        real next_sample() noexcept
        {
            return std::uniform_real_distribution<real>(0, 1)(rng_);
        }

    private:

        RNG rng_;
    };

    bool next_scattering_event(
        const Entity *entity, const Ray &r,
        EntityIntersection *ent_inct, // 用于输出r和entity的首个交点，返回true时有效
        MediumIntersection *med_inct, // 用于输出介质散射点，*is_med_event为true时有效
        bool *is_med_event,           // 是表面散射事件还是介质散射事件，返回true时有效
        real *pdf,                    // 散射事件的pdf，返回true时有效
        const Sample1 &sam)
    {
        if(!entity->closest_intersection(r, ent_inct))
            return false;

        auto medium = ent_inct->wr_medium();
        auto med_sample = medium->sample(r.o, r.d, r.t_min, ent_inct->t, sam);
        if(med_sample.invalid())
            return false;

        if(med_sample.inct.invalid())
        {
            *is_med_event = false;
            *pdf = med_sample.pdf;
        }
        else
        {
            *is_med_event = true;
            *med_inct = med_sample.inct;
            *pdf = med_sample.pdf;
        }

        return true;
    }
}

PathTracedBSSRDF::PathTracedBSSRDF(const EntityIntersection &xo, const Medium *medium)
    : BSSRDF(xo), medium_(medium)
{

}

BSSRDFSampleResult PathTracedBSSRDF::sample(const Vec3 &xo_wi, TransportMode mode, const Sample4 &sam, Arena &arena) const
{
    LocalSampler::Seed init_seed = LocalSampler::Seed(sam.u * (std::numeric_limits<LocalSampler::Seed>::max)());
    LocalSampler sampler(init_seed);



    // TODO
    return {};
}

AGZ_TRACER_END
