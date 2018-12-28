#include <Atrc/Lib/FilmFilter/GaussianFilter.h>

namespace Atrc
{
    
GaussianFilter::GaussianFilter(const Vec2 &radius, Real alpha) noexcept
    : FilmFilter(radius), alpha_(alpha),
      exp_(radius.Map([=](Real c) { return Exp(-alpha * c * c); }))
{

}

namespace
{
    Real Gaussian(Real d, Real expv, Real alpha)
    {
        return Max(Real(0), Real(Exp(-alpha * d * d) - expv));
    }
}

Real GaussianFilter::Eval(Real relX, Real relY) const noexcept
{
    return Gaussian(relX, exp_.x, alpha_) * Gaussian(relY, exp_.y, alpha_);
}

} // namespace Atrc
