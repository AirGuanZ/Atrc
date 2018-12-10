#pragma once

#include <queue>
#include <type_traits>

namespace Atrc
{

template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
class GridDivider
{
public:

    struct Grid
    {
        T xBegin, xEnd;
        T yBegin, yEnd;
    };

    static std::queue<Grid> Divide(const Grid &wholeGrid, T xGridSize, T yGridSize)
    {
        AGZ_ASSERT(xGridSize > 0 && yGridSize > 0);

        AGZ_ASSERT(wholeGrid.xBegin <= wholeGrid.xEnd);
        AGZ_ASSERT(wholeGrid.yBegin <= wholeGrid.yEnd);

        std::queue<Grid> ret;

        for(T yBegin = wholeGrid.yBegin; yBegin < wholeGrid.yEnd; yBegin += yGridSize)
        {
            T yEnd = (std::min)(yBegin + yGridSize, wholeGrid.yEnd);
            for(T xBegin = wholeGrid.xBegin; xBegin < wholeGrid.xEnd; xBegin += xGridSize)
            {
                T xEnd = (std::min)(xBegin + xGridSize, wholeGrid.xEnd);
                ret.push({ xBegin, xEnd, yBegin, yEnd });
            }
        }

        return ret;
    }
};

} // namespace Atrc
