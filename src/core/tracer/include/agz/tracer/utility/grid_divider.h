#pragma once

#include <cassert>
#include <type_traits>

#include <agz/common/common.h>

AGZ_TRACER_BEGIN

template<typename T>
class GridDivider
{
    static_assert(std::is_integral_v<T>);

public:

    struct Grid
    {
        T x_begin, x_end;
        T y_begin, y_end;
    };

    template<typename OutIterator>
    static void divide(const Grid &whole_grid, T x_grid_size, T y_grid_size, OutIterator out_it)
    {
        assert(x_grid_size > 0 && y_grid_size > 0);

        for(T y_begin = whole_grid.y_begin; y_begin < whole_grid.y_end; y_begin += y_grid_size)
        {
            T y_end = (std::min)(y_begin + y_grid_size, whole_grid.y_end);
            for(T x_begin = whole_grid.x_begin; x_begin < whole_grid.x_end; x_begin += x_grid_size)
            {
                T x_end = (std::min)(x_begin + x_grid_size, whole_grid.x_end);
                *out_it++ = Grid{ x_begin, x_end, y_begin, y_end };
            }
        }
    }
};

AGZ_TRACER_END
