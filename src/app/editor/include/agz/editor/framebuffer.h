#pragma once

#include <atomic>
#include <deque>
#include <mutex>
#include <shared_mutex>

#include <agz/editor/common.h>
#include <agz/tracer/core/render_target.h>

AGZ_EDITOR_BEGIN

class Framebuffer : public misc::uncopyable_t
{
public:

    struct Task
    {
        Image2D<Spectrum> value;
        Image2D<real> weight;

        Vec2 full_res;
        Rect2i pixel_range;
        int pixel_size = 1;

        int spp = 1;
    };

    Framebuffer(int width, int height, int init_task_size);

    ~Framebuffer();

    int get_tasks(int expected_task_count, std::vector<Task> &tasks);

    void merge_tasks(int task_count, Task *tasks);

    Image2D<math::color3b> get_image() const;

    Vec2i get_resolution() const noexcept;

private:

    Image2D<math::color3b> compute_image() const;

    // assert lock_shared(es_mutex)
    void merge_single_task(const Task &task);

    void rebuild_task_queue();

    int width_, height_;

    mutable std::shared_mutex es_mutex_;
    Image2D<Spectrum> value_;
    Image2D<real> weight_;
    Image2D<int> pixel_size_;

    std::mutex queue_mutex_;
    std::deque<Task> tasks_;

    mutable std::mutex output_mutex_;
    Image2D<math::color3b> output_;

    std::atomic<bool> exit_;
    std::thread output_updater_thread_;

    // 以下字段的写入由es_mutex_和queue_mutex_共同保护

    int init_task_size_;
};

AGZ_EDITOR_END
