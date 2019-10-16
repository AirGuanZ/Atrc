#pragma once

#include <memory>
#include <thread>
#include <vector>

#include <agz/rasterizer/common.h>
#include <agz/utility/thread.h>

AGZ_RASTERIZER_BEGIN

class RasterizerScheduler : public misc::uncopyable_t
{
public:

    struct ThreadLocalData
    {
        int thread_index;
        int thread_count;
    };

    /**
     * @brief 队列中存储的type erased任务对象
     */
    class ThreadTask
    {
    public:

        virtual ~ThreadTask() = default;

        virtual void run(const ThreadLocalData &thread_local_data) const noexcept = 0;
    };

    /**
     * @brief 外界需要继承此类型来定义单个任务
     */
    template<typename Data>
    class ThreadWorker
    {
    public:

        virtual ~ThreadWorker() = default;

        virtual void run(const ThreadLocalData &thread_local_data, const Data &data) const noexcept = 0;
    };

    using ThreadQueue = thread::blocking_queue_t<std::shared_ptr<ThreadTask>>;

    explicit RasterizerScheduler(int thread_count)
    {
        thread_count = thread::actual_worker_count(thread_count);
        thread_count_ = thread_count;

        threads_.reserve(thread_count);
        thread_local_data_.reserve(thread_count);
        task_queues_.reserve(thread_count);

        for(int i = 0; i < thread_count; ++i)
        {
            thread_local_data_.push_back({ i, thread_count });
            task_queues_.push_back(std::make_unique<ThreadQueue>());
        }

        for(int i = 0; i < thread_count; ++i)
            threads_.emplace_back(thread_func, task_queues_[i].get(), &thread_local_data_[i]);

        thread_local_data_.shrink_to_fit();
        task_queues_.shrink_to_fit();
        threads_.shrink_to_fit();
    }

    ~RasterizerScheduler()
    {
        sync();

        for(auto &q : task_queues_)
            q->stop();

        for(auto &t : threads_)
        {
            assert(t.joinable());
            t.join();
        }
    }

    template<typename Data>
    void add_task(std::shared_ptr<ThreadWorker<Data>> worker, std::shared_ptr<const Data> data)
    {
        auto task = std::make_shared<ThreadTaskImpl<Data>>(std::move(worker), std::move(data));
        for(auto &q : task_queues_)
            q->push(task);
    }

    void sync()
    {
        struct SyncTaskData
        {
            mutable int n = 0;
            mutable std::mutex mut;
            mutable std::condition_variable cond;
        };

        class SyncWorker : public ThreadWorker<SyncTaskData>
        {
        public:

            void run(const ThreadLocalData &, const SyncTaskData &data) const noexcept override
            {
                std::lock_guard lk(data.mut);
                if(!--data.n)
                    data.cond.notify_one();
            }
        };

        auto sync_data = std::make_shared<SyncTaskData>();
        auto sync_worker = std::make_shared<SyncWorker>();
        sync_data->n = thread_count_;

        add_task<SyncTaskData>(std::move(sync_worker), sync_data);

        std::unique_lock lk(sync_data->mut);
        while(sync_data->n > 0)
            sync_data->cond.wait(lk);
    }

private:

    static void thread_func(ThreadQueue *task_queue, const ThreadLocalData *thread_local_data)
    {
        for(;;)
        {
            auto opt_data = task_queue->pop_or_stop();
            if(!opt_data)
                break;
            opt_data->get()->run(*thread_local_data);
        }
    }

    template<typename Data>
    class ThreadTaskImpl : public ThreadTask
    {
        std::shared_ptr<ThreadWorker<Data>> worker_;
        std::shared_ptr<const Data> data_;

    public:

        ThreadTaskImpl(std::shared_ptr<ThreadWorker<Data>> worker, std::shared_ptr<const Data> data) noexcept
            : worker_(std::move(worker)), data_(std::move(data))
        {
            assert(worker_ != nullptr);
            assert(data_ != nullptr);
        }

        void run(const ThreadLocalData &thread_local_data) const noexcept override
        {
            worker_->run(thread_local_data, *data_);
        }
    };

    int thread_count_;

    std::vector<std::thread> threads_;
    std::vector<ThreadLocalData> thread_local_data_;
    std::vector<std::unique_ptr<ThreadQueue>> task_queues_;
};

AGZ_RASTERIZER_END
