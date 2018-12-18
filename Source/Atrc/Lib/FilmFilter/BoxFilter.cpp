#include <Atrc/Lib/FilmFilter/BoxFilter.h>

namespace Atrc
{

BoxFilter::BoxFilter(const Vec2 &radius) noexcept
    : FilmFilter(radius)
{

}

Spectrum BoxFilter::Eval([[maybe_unused]] Real relX, [[maybe_unused]] Real relY) const noexcept
{
    return Spectrum(Real(1));
}

} // namespace Atrc
