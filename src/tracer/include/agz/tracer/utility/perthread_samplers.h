#pragma once

#include <agz/tracer/core/sampler.h>

AGZ_TRACER_BEGIN

class PerThreadNativeSamplers : public misc::uncopyable_t
{
public:

    PerThreadNativeSamplers();

    PerThreadNativeSamplers(size_t threadCount, const NativeSampler &parent);

    PerThreadNativeSamplers(PerThreadNativeSamplers &&other) noexcept;

    PerThreadNativeSamplers &operator=(PerThreadNativeSamplers &&other) noexcept;

    ~PerThreadNativeSamplers();

    void swap(PerThreadNativeSamplers &other) noexcept;

    NativeSampler *get_sampler(size_t threadIdx) noexcept;

    NativeSampler *operator[](size_t threadIdx) noexcept;

private:

    static constexpr size_t STORAGE_ALIGN = 64;
    static constexpr size_t STORAGE_SIZE  = 64;

    struct SamplerStorage
    {
        SamplerStorage(int seed, bool use_time_seed);

        NativeSampler sampler;
        char cache_pad[STORAGE_SIZE - sizeof(NativeSampler)] = {};
    };

    static_assert(sizeof(SamplerStorage) == STORAGE_SIZE);

    size_t count_;
    SamplerStorage *samplers_;
};

inline PerThreadNativeSamplers::PerThreadNativeSamplers()
    : count_(0), samplers_(nullptr)
{
    
}

inline PerThreadNativeSamplers::PerThreadNativeSamplers(
    size_t threadCount, const NativeSampler &parent)
    : count_(threadCount)
{
    samplers_ = reinterpret_cast<SamplerStorage *>(
        alloc::aligned_alloc(
            sizeof(SamplerStorage) * threadCount, STORAGE_ALIGN));

    size_t i = 0;
    try
    {
        for(; i < threadCount; ++i)
        {
            new(samplers_ + i) SamplerStorage(
                static_cast<int>(parent.get_seed() + i), false);
        }
    }
    catch(...)
    {
        alloc::call_destructor(samplers_, i);
        alloc::aligned_free(samplers_);
        throw;
    }
}

inline PerThreadNativeSamplers::PerThreadNativeSamplers(
    PerThreadNativeSamplers &&other) noexcept
    : PerThreadNativeSamplers()
{
    swap(other);
}

inline PerThreadNativeSamplers &PerThreadNativeSamplers::operator=(
    PerThreadNativeSamplers &&other) noexcept
{
    swap(other);
    return *this;
}

inline PerThreadNativeSamplers::~PerThreadNativeSamplers()
{
    if(!samplers_)
        return;
    alloc::call_destructor(samplers_, count_);
    alloc::aligned_free(samplers_);
}

inline void PerThreadNativeSamplers::swap(PerThreadNativeSamplers &other) noexcept
{
    std::swap(count_, other.count_);
    std::swap(samplers_, other.samplers_);
}

inline NativeSampler *PerThreadNativeSamplers::get_sampler(
    size_t threadIdx) noexcept
{
    return &samplers_[threadIdx].sampler;
}

inline NativeSampler *PerThreadNativeSamplers::operator[](
    size_t threadIdx) noexcept
{
    return get_sampler(threadIdx);
}

inline PerThreadNativeSamplers::SamplerStorage::SamplerStorage(
    int seed, bool use_time_seed)
    : sampler(seed, use_time_seed)
{
    
}

AGZ_TRACER_END
