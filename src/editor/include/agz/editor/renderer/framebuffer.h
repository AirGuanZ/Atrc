#pragma once

#include <atomic>
#include <deque>
#include <mutex>
#include <shared_mutex>
#include <thread>

#include <QObject>

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

class Framebuffer : public QObject, public misc::uncopyable_t
{
public:

    struct Grid
    {
        Image2D<Spectrum> value;
        Image2D<real> weight;
    };

    struct Task
    {
        mutable Box<Grid> grid;

        Vec2 full_res;
        Rect2i pixel_range;
        int pixel_size = 1;

        int spp = 1;
    };

    Framebuffer(
        int width, int height, int task_grid_size, int init_pixel_size = 3);

    ~Framebuffer();

    void start();

    int get_tasks(int expected_task_count, std::vector<Task> &tasks);

    void merge_tasks(int task_count, Task *tasks);

    Image2D<Spectrum> get_image() const;

    Vec2i get_resolution() const noexcept;

private:

    Image2D<Spectrum> compute_image() const;

    // assert lock_shared(es_mutex)
    void merge_single_task(const Task &task);

    void merge_full_res_task(const Task &task);

    void merge_partial_res_task(const Task &task);

    void build_task_queue();

    const int width_, height_;
    const int task_grid_size_;
    const int init_pixel_size_;

    // value/weight buffer

    mutable std::shared_mutex es_mutex_;

    Image2D<Spectrum> value_;
    Image2D<real>     weight_;
    Image2D<int>      pixel_size_;

    // task queue

    std::mutex queue_mutex_;
    std::deque<Task> tasks_;

    // output image

    mutable std::mutex output_mutex_;
    Image2D<Spectrum> output_;

    // thread updating output image

    std::atomic<bool> exit_;
    std::thread output_updater_thread_;
};

AGZ_EDITOR_END
