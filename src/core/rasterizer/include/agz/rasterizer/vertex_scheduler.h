#pragma once

#include <memory>
#include <thread>
#include <vector>

#include <agz/rasterizer/common.h>
#include <agz/utility/thread.h>

AGZ_RASTERIZER_BEGIN

class VertexScheduler : public misc::uncopyable_t
{
public:

    class ThreadTask
    {
    public:

        virtual ~ThreadTask() = default;

        virtual void run() const noexcept = 0;
    };

    template<typename Data>
    class ThreadWorker
    {
    public:

        virtual ~ThreadWorker() = default;

        virtual void run(const Data &data) const noexcept = 0;
    };

    using TaskQueue = thread::blocking_queue_t<std::unique_ptr<ThreadTask>>;

    explicit VertexScheduler(int thread_count)
    {
        thread_count = thread::actual_worker_count(thread_count);
        thread_count_ = thread_count;

        task_queue_ = std::make_unique<TaskQueue>();

        threads_.reserve(thread_count);
        for(int i = 0; i < thread_count; ++i)
            threads_.emplace_back(thread_func, task_queue_.get());
    }

    ~VertexScheduler()
    {
        task_queue_->stop();
        for(auto &t : threads_)
        {
            assert(t.joinable());
            t.join();
        }
    }

    template<typename Data>
    void add_task(std::shared_ptr<ThreadWorker<Data>> worker, std::shared_ptr<Data> data)
    {
        auto task = std::make_unique<ThreadTaskImpl<Data>>(std::move(worker), std::move(data));
        task_queue_->push(std::move(task));
    }

    void sync()
    {
        task_queue_->stop();
        for(auto &t : threads_)
        {
            assert(t.joinable());
            t.join();
        }
        threads_.clear();
        threads_.reserve(thread_count_);

        task_queue_ = std::make_unique<TaskQueue>();
        for(int i = 0; i < thread_count_; ++i)
            threads_.emplace_back(thread_func, task_queue_.get());
    }

private:

    static void thread_func(TaskQueue *task_queue)
    {
        for(;;)
        {
            auto opt_data = task_queue->pop_or_stop();
            if(!opt_data)
                break;
            opt_data->get()->run();
        }
    }

    template<typename Data>
    class ThreadTaskImpl : public ThreadTask
    {
        std::shared_ptr<ThreadWorker<Data>> worker_;
        std::shared_ptr<const Data> data_;

    public:

        ThreadTaskImpl(std::shared_ptr<ThreadWorker<Data>> worker, std::shared_ptr<Data> data)
            : worker_(std::move(worker)), data_(std::move(data))
        {

        }

        void run() const noexcept override
        {
            worker_->run(*data_);
        }
    };

    int thread_count_;

    std::vector<std::thread> threads_;
    std::unique_ptr<TaskQueue> task_queue_;
};

AGZ_RASTERIZER_END
