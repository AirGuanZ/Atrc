#pragma once

#include <Atrc/Lib/Core/TFilm.h>

namespace Atrc
{

class BoxFilter : public FilmFilter
{
    Vec2 radius_;

public:

    explicit BoxFilter(const Vec2 &radius) noexcept;

    Real Eval(Real relX, Real relY) const noexcept override;
};

} // namespace Atrc
