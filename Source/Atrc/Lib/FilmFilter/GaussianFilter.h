#pragma once

#include <Atrc/Lib/Core/TFilm.h>

namespace Atrc
{
    
class GaussianFilter : public FilmFilter
{
    Real alpha_;
    Vec2 exp_;

public:

    GaussianFilter(const Vec2 &radius, Real alpha) noexcept;

    Real Eval(Real relX, Real relY) const noexcept override;
};

} // namespace Atrc
