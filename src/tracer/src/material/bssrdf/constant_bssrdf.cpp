#include "./constant_bssrdf.h"

AGZ_TRACER_BEGIN

ConstantBSSRDF::ConstantBSSRDF(
    const EntityIntersection& inct, const Coord &geometry_coord, const Coord &shading_coord,
    real eta, const Spectrum &A, const Spectrum &d) noexcept
    : SeparableBSSRDF(inct, geometry_coord, shading_coord, eta),
    A_(A), d_(d)
{

}

Spectrum ConstantBSSRDF::distance_factor(real distance) const noexcept
{
    return A_ / (d_ * d_ * PI_r);
}

real ConstantBSSRDF::sample_distance(int channel, const Sample1 &sam) const noexcept
{
    return std::sqrt(sam.u) * d_[channel];
}

real ConstantBSSRDF::pdf_distance(int channel, real dist) const noexcept
{
    if(dist > d_[channel])
        return 0;
    return 2 * dist / math::sqr(d_[channel]);
}

AGZ_TRACER_END
