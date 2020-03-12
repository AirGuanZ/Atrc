#pragma once

#include <thread>
#include <vector>

#include <agz/tracer/common.h>

AGZ_TRACER_BEGIN

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
    int thread_count, int width, int height, int grid_size, const Func &func)
{
    const int x_task_count = (width + grid_size - 1) / grid_size;
    const int y_task_count = (height + grid_size - 1) / grid_size;
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

            const int x_beg = x_idx * grid_size;
            const int y_beg = y_idx * grid_size;
            const int x_end = (std::min)(width, x_beg + grid_size);
            const int y_end = (std::min)(height, y_beg + grid_size);

            const Rect2i grid = { { x_beg, y_beg }, { x_end, y_end } };
            if(!func(thread_index, grid))
                return;
        }
    };

    std::vector<std::thread> threads;
    for(int i = 0; i < thread_count; ++i)
        threads.emplace_back(worker_func, i);

    for(auto &t : threads)
        t.join();
}

AGZ_TRACER_END
