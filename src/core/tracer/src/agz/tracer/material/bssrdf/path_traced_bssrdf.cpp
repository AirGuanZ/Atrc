#include <random>

#include "./path_traced_bssrdf.h"

AGZ_TRACER_BEGIN

PathTracedBSSRDF::PathTracedBSSRDF(
    const EntityIntersection &inct, const Vec3 &xo_wi,
    const Spectrum &sigma_a, const Spectrum &sigma_s, real g)
    : BSSRDF(inct), xo_wi_(xo_wi),
      sigma_s_(sigma_s), sigma_t_(sigma_a + sigma_s),
      phase_function_(g, sigma_s, !sigma_t_ ? Spectrum(1) : sigma_s / sigma_t_)
{
    
}

BSSRDFSampleResult PathTracedBSSRDF::sample(TransportMode mode, const Sample4 &sam, Arena &arena) const
{
    using RNG = std::minstd_rand;
    RNG rng(RNG::result_type(sam.u * RNG::max()));
    std::uniform_real_distribution<real> dis(0, 1);
    auto rand = [&] { return dis(rng); };

    // TODO

    return {};
}

AGZ_TRACER_END
