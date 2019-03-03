#pragma once

#include <Atrc/Lib/Core/TFilm.h>

namespace Atrc
{

class GaussianFilter : public FilmFilter
{
    Real alpha_;
    Vec2 exp_;

public:

    GaussianFilter(const Vec2 &radius, Real alpha) noexcept
        : FilmFilter(radius), alpha_(alpha),
        exp_(radius.Map([=](Real c) { return Exp(-alpha * c * c); }))
    {

    }

    Real Eval(Real relX, Real relY) const noexcept override
    {
        static auto Gaussian = [](Real d, Real expv, Real alpha)
        {
            return Max(Real(0), Real(Exp(-alpha * d * d) - expv));
        };
        return Gaussian(relX, exp_.x, alpha_) * Gaussian(relY, exp_.y, alpha_);
    }
};

} // namespace Atrc
