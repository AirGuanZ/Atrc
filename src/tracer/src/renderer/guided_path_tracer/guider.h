#pragma once

#include <mutex>
#include <queue>
#include <thread>

#include "./guider_impl/stree.h"

AGZ_TRACER_BEGIN

namespace pgpt
{

/**
 * @brief 单个radiance值的记录
 */
struct Record
{
    Vec3 pos;
    Vec3 dir;
    Spectrum radiance;
};

using RecordBatch = std::vector<Record>;

/**
 * @brief 用来近似记录光场分布的spatial + directional tree
 * 
 * 线程安全：可同时调用多个const成员函数或一个非const成员函数
 */
class SDTree : public misc::uncopyable_t
{
    STree stree_;

public:

    explicit SDTree(const AABB &world_bound)
        : stree_(world_bound)
    {

    }

    /**
     * @brief 完成一次训练迭代，至少迭代一次后才能用于采样
     */
    void iterate(real dtree_rho, uint32_t stree_threshold)
    {
        stree_.iterate(dtree_rho, stree_threshold);
    }

    /**
     * @brief 完成最后一次训练迭代，之后就只能采样不能训练了
     */
    void iterate_end()
    {
        stree_.iterate_end();
    }

    /**
     * @brief 取得pos点对应的方向采样器，至少训练一次后才能使用
     */
    const DSampler *dsampler(const Vec3 &pos) const
    {
        return stree_.direction_sampler(pos);
    }

    /**
     * @brief 添加一条训练数据
     */
    void record(const Vec3 &pos, const Vec3 &dir, const Spectrum &radiance)
    {
        stree_.record(pos, dir, radiance.lum());
    }

    /**
     * @brief 添加一组训练数据
     */
    void record(const RecordBatch &record_batch)
    {
        for(auto &rcd : record_batch)
            record(rcd.pos, rcd.dir, rcd.radiance);
    }
};

/*
支持以下操作：
    add_batch  : thread safe
    dsampler   : thread safe
    iterate    : called from main thread
    iterate_end: called from main thread
*/
class Guider : public misc::uncopyable_t
{
    std::atomic<bool> stop_recording_flag_;

    std::mutex batch_queue_mutex_;
    std::queue<RecordBatch> batch_queue_;

    SDTree tree_;

    std::thread recording_thread_;

    static void record_func(Guider *p_this)
    {
        for(;;)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            std::vector<RecordBatch> batches;

            {
                std::lock_guard lk(p_this->batch_queue_mutex_);
                while(!p_this->batch_queue_.empty())
                {
                    batches.push_back(std::move(p_this->batch_queue_.front()));
                    p_this->batch_queue_.pop();
                }
            }

            for(auto &batch : batches)
                p_this->tree_.record(batch);

            if(p_this->stop_recording_flag_)
                return;
        }
    }

    void start_recording()
    {
        stop_recording_flag_ = false;
        recording_thread_ = std::thread(record_func, this);
    }

    void stop_recording()
    {
        assert(recording_thread_.joinable());
        stop_recording_flag_ = true;
        recording_thread_.join();

        std::lock_guard lk(batch_queue_mutex_);
        while(!batch_queue_.empty())
        {
            tree_.record(batch_queue_.front());
            batch_queue_.pop();
        }
    }

public:

    explicit Guider(const AABB &world_bound)
        : stop_recording_flag_(false), tree_(world_bound)
    {
        start_recording();
    }

    ~Guider()
    {
        stop_recording_flag_ = true;
        if(recording_thread_.joinable())
            recording_thread_.join();
    }

    void iterate(real dtree_rho, uint32_t stree_threshold)
    {
        stop_recording();
        tree_.iterate(dtree_rho, stree_threshold);
        start_recording();
    }

    void iterate_end()
    {
        stop_recording();
        tree_.iterate_end();
    }

    const DSampler *dsampler(const Vec3 &pos) const
    {
        return tree_.dsampler(pos);
    }

    void add_batch(RecordBatch &&batch)
    {
        std::lock_guard lk(batch_queue_mutex_);
        batch_queue_.push(std::move(batch));
    }
};

class RecordBatchBuilder
{
    size_t batch_size_;
    RecordBatch batch_;

    Guider *guider_;

public:

    RecordBatchBuilder(size_t batch_size, Guider &guider)
        : batch_size_(batch_size), guider_(&guider)
    {
        assert(batch_size_ > 0);
    }

    void add(const Record &rcd)
    {
        batch_.push_back(rcd);
        if(batch_.size() >= batch_size_)
        {
            guider_->add_batch(std::move(batch_));
            batch_.clear();
        }
    }

    void end()
    {
        if(!batch_.empty())
        {
            guider_->add_batch(std::move(batch_));
            batch_.clear();
        }
    }
};
    
} // namespace pgpt

AGZ_TRACER_END
