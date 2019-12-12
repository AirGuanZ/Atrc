#pragma once

#include <agz/tracer/common.h>

AGZ_TRACER_BEGIN

/**
 * @brief 采样器接口，用于在渲染过程中提供随机数
 */
class Sampler
{
public:

    virtual ~Sampler() = default;

    /**
     * @brief 将新seed和内部seed结合clone一个新的sampler
     */
    virtual Sampler *clone(int seed, Arena &arena) const = 0;

    virtual Sample1 sample1() noexcept = 0;

    virtual Sample2 sample2() noexcept;
    virtual Sample3 sample3() noexcept;
    virtual Sample4 sample4() noexcept;
    virtual Sample5 sample5() noexcept;

    template<int N>
    SampleN<N> SampleN() noexcept;

    virtual void start_pixel(int x, int y) = 0;

    virtual bool next_sample() = 0;

    virtual int get_spp() const noexcept = 0;
};

inline Sample2 Sampler::sample2() noexcept
{
    real u = sample1().u;
    real v = sample1().u;
    return { u, v };
}

inline Sample3 Sampler::sample3() noexcept
{
    real u = sample1().u;
    real v = sample1().u;
    real w = sample1().u;
    return { u, v, w };
}

inline Sample4 Sampler::sample4() noexcept
{
    real u = sample1().u;
    real v = sample1().u;
    real w = sample1().u;
    real r = sample1().u;
    return { u, v, w, r };
}

inline Sample5 Sampler::sample5() noexcept
{
    real u = sample1().u;
    real v = sample1().u;
    real w = sample1().u;
    real r = sample1().u;
    real s = sample1().u;
    return { u, v, w, r, s };
}

template<int N>
SampleN<N> Sampler::SampleN() noexcept
{
    tracer::SampleN<N> ret;
    for(int i = 0; i < N; ++i)
        ret.u[i] = sample1().u;
    return ret;
}

AGZ_TRACER_END
