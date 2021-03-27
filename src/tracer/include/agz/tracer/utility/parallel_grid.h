#pragma once

#include <agz/tracer/common.h>
#include <agz-utils/thread.h>

AGZ_TRACER_BEGIN

/**
 * @brief divide [0, total_width) to equal-sized ranges
 *  and exec func on them parallelly
 *
 * func interface: bool func(int thread_index, int beg, int end)
 *
 * if any one func call returns false, that worker thread is stopped immediately
 */
template<typename Func>
void parallel_for_1d_grid(
    int thread_count, int total_width, int grid_size,
    thread::thread_group_t &threads, Func &&func)
{
    const int task_count = (total_width + grid_size - 1) / grid_size;

    std::atomic<int> next_task = 0;

    auto worker_func = [&](int thread_index)
    {
        for(;;)
        {
            const int task_idx = next_task++;
            if(task_idx >= task_count)
                return;

            const int beg = task_idx * grid_size;
            const int end = (std::min)(beg + grid_size, total_width);

            if constexpr(
                std::is_convertible_v<
                decltype(func(thread_index, beg, end)), bool>)
            {
                if(!func(thread_index, beg, end))
                    return;
            }
            else
                func(thread_index, beg, end);
        }
    };

    threads.run(thread_count, worker_func);
}

/**
 * @brief divide [0, width) * [0, height) to equal-sized grids
 *  and exec func on them parallelly
 *
 * func interface: bool func(int thread_index, Rect2i grid)
 *
 * if any one func call returns false, that worker thread is stopped immediately
 */
template<typename Func>
void parallel_for_2d_grid(
    int thread_count, int width, int height,
    int grid_size_x, int grid_size_y,
    thread::thread_group_t &threads, Func &&func)
{
    const int x_task_count = (width  + grid_size_x - 1) / grid_size_x;
    const int y_task_count = (height + grid_size_y - 1) / grid_size_y;
    const int total_task_count = x_task_count * y_task_count;

    std::atomic<int> next_task = 0;

    auto worker_func = [&](int thread_index)
    {
        for(;;)
        {
            const int task_idx = next_task++;
            if(task_idx >= total_task_count)
                return;

            const int y_idx = task_idx / x_task_count;
            const int x_idx = task_idx % x_task_count;

            const int x_beg = x_idx * grid_size_x;
            const int y_beg = y_idx * grid_size_y;
            const int x_end = (std::min)(width,  x_beg + grid_size_x);
            const int y_end = (std::min)(height, y_beg + grid_size_y);

            const Rect2i grid = { { x_beg, y_beg }, { x_end, y_end } };

            if constexpr(
                std::is_convertible_v<
                decltype(func(thread_index, grid)), bool>)
            {
                if(!func(thread_index, grid))
                    return;
            }
            else
                func(thread_index, grid);
        }
    };

    threads.run(thread_count, worker_func);
}

/**
 * @brief parallal_for_1d_grid without thread_group
 */
template<typename Func>
void parallel_for_1d_grid(
    int thread_count, int total_width, int grid_size, Func &&func)
{
    thread::thread_group_t threads;
    parallel_for_1d_grid(
        thread_count, total_width, grid_size,
        threads, std::forward<Func>(func));
}

/**
 * @brief parallal_for_2d_grid without thread_group
 */
template<typename Func>
void parallel_for_2d_grid(
    int thread_count, int width, int height,
    int grid_size_x, int grid_size_y, Func &&func)
{
    thread::thread_group_t threads;
    parallel_for_2d_grid(
        thread_count, width, height, grid_size_x, grid_size_y,
        threads, std::forward<Func>(func));
}

AGZ_TRACER_END
