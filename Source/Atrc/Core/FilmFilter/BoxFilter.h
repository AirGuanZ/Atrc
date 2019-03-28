#pragma once

#include <Atrc/Core/Core/TFilm.h>

namespace Atrc
{

class BoxFilter : public FilmFilter
{
public:

    explicit BoxFilter(const Vec2 &radius) noexcept
        : FilmFilter(radius)
    {

    }

    Real Eval(Real relX, Real relY) const noexcept override
    {
        return 1;
    }
};

} // namespace Atrc
