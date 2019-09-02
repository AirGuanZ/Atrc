#include "./normalized_diffusion.h"

AGZ_TRACER_BEGIN

NormalizedDiffusionBSSRDF::NormalizedDiffusionBSSRDF(const EntityIntersection& inct, const Coord &geometry_coord, const Coord &shading_coord, real eta, const Spectrum &A, const Spectrum &d) noexcept
    : SeparableBSSRDF(inct, geometry_coord, shading_coord, eta),
      A_(A), d_(d)
{

}

Spectrum NormalizedDiffusionBSSRDF::distance_factor(real distance) const noexcept
{
    distance = std::max(distance, EPS);
    Spectrum a = math::exp(-Spectrum(distance) / d_);
    Spectrum b = math::exp(-Spectrum(distance) / (real(3) * d_));
    Spectrum dem = 4 * PI_r * d_ * distance;
    return A_ * (a + b) / dem;
}

real NormalizedDiffusionBSSRDF::sample_distance(int channel, const Sample1 &sam) const noexcept
{
    real u = sam.u;
    if(u <= real(0.25))
    {
        u = (std::min)(u * 4, 1 - EPS);
        return d_[channel] * std::log(1 / (1 - u));
    }
    u = (std::min)((u - real(0.25)) / real(0.75), 1 - EPS);
    return 3 * d_[channel] * std::log(1 / (1 - u));
}

real NormalizedDiffusionBSSRDF::pdf_distance(int channel, real dist) const noexcept
{
    dist = (std::max)(dist, EPS);
    real a = std::exp(-dist / d_[channel]) / d_[channel];
    real b = std::exp(-dist / (3 * d_[channel])) / (3 * d_[channel]);
    return real(0.25) * a + real(0.75) * b;
}

AGZ_TRACER_END
