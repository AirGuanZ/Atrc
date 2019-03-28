#pragma once

#include <queue>
#include <type_traits>

#include <AGZUtils/Utils/Math.h>

namespace Atrc
{

template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
class GridDivider
{
public:

    using Grid = AGZ::Math::Rect<T>;

    template<typename OutIterator>
    static void Divide(const Grid &wholeGrid, T xGridSize, T yGridSize, OutIterator outIt)
    {
        AGZ_ASSERT(xGridSize > 0 && yGridSize > 0);

        for(T yBegin = wholeGrid.low.y; yBegin < wholeGrid.high.y; yBegin += yGridSize)
        {
            T yEnd = (std::min)(yBegin + yGridSize, wholeGrid.high.y);
            for(T xBegin = wholeGrid.low.x; xBegin < wholeGrid.high.x; xBegin += xGridSize)
            {
                T xEnd = (std::min)(xBegin + xGridSize, wholeGrid.high.x);
                *outIt++ = Grid{ { xBegin, yBegin }, { xEnd, yEnd} };
            }
        }
    }
};

} // namespace Atrc
