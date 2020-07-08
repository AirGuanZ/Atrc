#include <chrono>
#include <random>

#include <agz/editor/renderer/framebuffer.h>

AGZ_EDITOR_BEGIN

Framebuffer::Framebuffer(
    int width, int height, int task_grid_size, int init_pixel_size)
    : width_(width), height_(height),
      task_grid_size_(task_grid_size), init_pixel_size_(init_pixel_size)
{
    value_      = Image2D<Spectrum>(height, width);
    weight_     = Image2D<real>(height, width);
    pixel_size_ = Image2D<int>(height, width);

    build_task_queue();

    exit_ = false;
}

Framebuffer::~Framebuffer()
{
    exit_ = true;
    if(output_updater_thread_.joinable())
        output_updater_thread_.join();
}

void Framebuffer::start()
{
    output_updater_thread_ = std::thread([this]
    {
        constexpr std::chrono::milliseconds wait_ms(100);

        while(!exit_)
        {
            std::this_thread::sleep_for(wait_ms);
            if(exit_)
                return;

            if(auto new_output = compute_image(); new_output.is_available())
            {
                {
                    std::lock_guard lk(output_mutex_);
                    output_.swap(new_output);
                }
            }
        }
    });
}

int Framebuffer::get_tasks(int expected_task_count, std::vector<Task> &tasks)
{
    std::lock_guard lk(queue_mutex_);

    tasks.reserve(tasks.size() + expected_task_count);

    int ret = 0;
    for(int i = 0; i < expected_task_count; ++i)
    {
        if(tasks_.empty())
            break;

        auto &top = tasks_.front();

        Task new_task;
        new_task.grid        = std::move(top.grid);
        new_task.full_res    = top.full_res;
        new_task.pixel_range = top.pixel_range;
        new_task.pixel_size  = top.pixel_size;
        new_task.spp         = top.spp;

        tasks.push_back(std::move(new_task));
        tasks_.pop_front();
        ++ret;
    }

    return ret;
}

void Framebuffer::merge_tasks(int task_count, Task *tasks)
{
    {
        es_mutex_.lock_shared();
        AGZ_SCOPE_GUARD({ es_mutex_.unlock_shared(); });

        for(int i = 0; i < task_count; ++i)
            merge_single_task(tasks[i]);
    }

    {
        std::lock_guard lk(queue_mutex_);

        for(int i = 0; i < task_count; ++i)
        {
            auto &task = tasks[i];

            if(task.pixel_size == 1)
            {
                if(task.iter_cnt++ >= 12)
                    task.spp = (std::min)(task.spp + 1, 16);
                else
                    task.spp = 1;

                tasks_.push_back(std::move(task));
                continue;
            }

            const int new_x_min = task.pixel_range.low.x * 2;
            const int new_y_min = task.pixel_range.low.y * 2;
            
            const int old_x_size = task.pixel_range.high.x
                                 - task.pixel_range.low.x + 1;
            const int old_y_size = task.pixel_range.high.y
                                 - task.pixel_range.low.y + 1;

            const int new_x_size = old_x_size * 2;
            const int new_y_size = old_y_size * 2;

            const int new_x_max = new_x_min + new_x_size - 1;
            const int new_y_max = new_y_min + new_y_size - 1;

            const int new_pixel_size = task.pixel_size / 2;

            Task new_task;
            new_task.grid         = std::move(task.grid);
            new_task.grid->value  = Image2D<Spectrum>(new_y_size, new_x_size);
            new_task.grid->weight = Image2D<real>(new_y_size, new_x_size);
            new_task.full_res     = {
                real(width_) / new_pixel_size, real(height_) / new_pixel_size };
            new_task.pixel_range  = {
                { new_x_min, new_y_min }, { new_x_max, new_y_max } };
            new_task.pixel_size   = task.pixel_size / 2;
            new_task.spp          = 1;

            tasks_.push_back(std::move(new_task));
        }
    }
}

Image2D<Spectrum> Framebuffer::get_image() const
{
    std::lock_guard lk(output_mutex_);
    if(output_.is_available())
        return output_;
    return {};
}

Vec2i Framebuffer::get_resolution() const noexcept
{
    return { width_, height_ };
}

Image2D<Spectrum> Framebuffer::compute_image() const
{
    Image2D<Spectrum> value;
    Image2D<real>     weight;
    {
        std::lock_guard lk(es_mutex_);
        value  = value_;
        weight = weight_;
    }

    Image2D<Spectrum> image(value.height(), value.width());
    for(int y = 0; y < image.height(); ++y)
    {
        if(exit_)
            return {};

        for(int x = 0; x < image.width(); ++x)
        {
            const real w = weight(y, x);
            if(w <= 0)
                return {};

            image(y, x) = value(y, x) / w;
        }
    }

    return image;
}

void Framebuffer::merge_single_task(const Task &task)
{
    if(task.pixel_size == 1)
    {
        merge_full_res_task(task);
        return;
    }
    merge_partial_res_task(task);
}

void Framebuffer::merge_full_res_task(const Task &task)
{
    assert(task.pixel_size == 1);

    for(int y = task.pixel_range.low.y, ly = 0;
        y <= task.pixel_range.high.y; ++y, ++ly)
    {
        for(int x = task.pixel_range.low.x, lx = 0;
            x <= task.pixel_range.high.x; ++x, ++lx)
        {
            if(pixel_size_(y, x) != 1)
            {
                value_(y, x)  = task.grid->value(ly, lx);
                weight_(y, x) = task.grid->weight(ly, lx);

                assert(task.spp == 1);
                pixel_size_(y, x) = 1;
            }
            else
            {
                value_(y, x)  += task.grid->value(ly, lx);
                weight_(y, x) += task.grid->weight(ly, lx);
            }
        }
    }
}

void Framebuffer::merge_partial_res_task(const Task &task)
{
    int y_base = task.pixel_range.low.y * task.pixel_size;

    for(int ly = 0; ly < task.grid->value.height(); ++ly)
    {
        int x_base = task.pixel_range.low.x * task.pixel_size;

        for(int lx = 0; lx < task.grid->value.width(); ++lx)
        {
            for(int y_offset = 0; y_offset < task.pixel_size; ++y_offset)
            {
                const int py = y_base + y_offset;
                for(int x_offset = 0; x_offset < task.pixel_size; ++x_offset)
                {
                    const int px = x_base + x_offset;

                    if(pixel_size_(py, px) != 1)
                    {
                        value_(py, px)      = task.grid->value(ly, lx);
                        weight_(py, px)     = task.grid->weight(ly, lx);
                        pixel_size_(py, px) = task.pixel_size;
                    }
                    else
                    {
                        value_(py, px)  += task.grid->value(ly, lx);
                        weight_(py, px) += task.grid->weight(ly, lx);
                    }
                }
            }
            x_base += task.pixel_size;
        }
        y_base += task.pixel_size;
    }
}

void Framebuffer::build_task_queue()
{
    // assert(queue_mutex is locked);

    const int x_task_count = (width_ + task_grid_size_  - 1) / task_grid_size_;
    const int y_task_count = (height_ + task_grid_size_ - 1) / task_grid_size_;

    std::vector<Task> tasks;
    tasks.reserve(x_task_count * y_task_count);

    for(int y = 0; y < y_task_count; ++y)
    {
        const int y_min = y * task_grid_size_;
        
        for(int x = 0; x < x_task_count; ++x)
        {
            const int x_min = x * task_grid_size_;
            
            int x_low = x_min;
            int y_low = y_min;
            int x_rng = (std::min)(x_min + task_grid_size_, width_) - x_min;
            int y_rng = (std::min)(y_min + task_grid_size_, height_) - y_min;

            int pixel_size = 1;

            for(int i = 2; i <= init_pixel_size_; ++i)
            {
                if(x_rng <= 1 || y_rng <= 1 ||
                   (x_low & 1) || (x_rng & 1) ||
                   (y_low & 1) || (y_rng & 1))
                    break;

                x_low /= 2, y_low /= 2;
                x_rng /= 2, y_rng /= 2;

                pixel_size *= 2;
            }

            Task task;
            task.grid = newBox<Grid>();

            task.grid->value  = Image2D<Spectrum>(y_rng, x_rng);
            task.grid->weight = Image2D<real>(y_rng, x_rng);
            task.full_res      = {
                real(width_) / pixel_size, real(height_) / pixel_size };
            task.pixel_range   = {
                { x_low, y_low }, { x_low + x_rng - 1, y_low + y_rng - 1} };
            task.pixel_size    = pixel_size;
            task.spp           = 1;

            tasks.push_back(std::move(task));
        }
    }

    std::default_random_engine rng(
        static_cast<std::default_random_engine::result_type>(
            std::chrono::high_resolution_clock::now()
                .time_since_epoch().count()));
    std::shuffle(tasks.begin(), tasks.end(), rng);

    for(auto &task : tasks)
        tasks_.push_back(std::move(task));
}

AGZ_EDITOR_END
