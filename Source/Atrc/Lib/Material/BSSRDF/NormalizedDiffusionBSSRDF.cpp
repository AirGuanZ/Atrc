#include <Atrc/Lib/Material/BSSRDF/NormalizedDiffusionBSSRDF.h>

namespace Atrc
{

NormalizedDiffusionBSSRDF::NormalizedDiffusionBSSRDF(
    const Intersection &inct, Real eta, const Spectrum &A, Real mfp) noexcept
    : SeparableBSSRDF(inct, eta), A_(A), l_(mfp)
{
    s_ = -A + Real(1.9) + Real(3.5) * (A - Real(0.8)).Map([](Real c) { return c * c; });
}

Spectrum NormalizedDiffusionBSSRDF::Sr(Real distance) const noexcept
{
    distance /= l_;
    Spectrum a = (-s_ * distance).Map(Exp<Real>) + (-s_ * distance / 3).Map(Exp<Real>);
    Spectrum b(8 * PI * distance);
    return A_ * s_ * a / b;
}

} // namespace Atrc
