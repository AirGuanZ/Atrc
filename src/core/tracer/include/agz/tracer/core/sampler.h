#pragma once

#include <agz/common/math.h>
#include <agz/tracer/core/object.h>

AGZ_TRACER_BEGIN

/**
 * @brief 采样器接口，用于在渲染过程中提供随机数
 */
class Sampler : public obj::Object
{
public:

    using Object::Object;

    virtual Sample1 sample1() noexcept = 0;

    virtual Sampler *clone(int seed, Arena &arena) const = 0;

    virtual Sample2 sample2() noexcept;
    virtual Sample3 sample3() noexcept;
    virtual Sample4 sample4() noexcept;
    virtual Sample5 sample5() noexcept;

    template<int N>
    SampleN<N> SampleN() noexcept;

    virtual void start_pixel(int x, int y) = 0;

    virtual bool next_sample() = 0;
};

AGZT_INTERFACE(Sampler)

inline Sample2 Sampler::sample2() noexcept
{
    Sample2 ret;
    ret.u = sample1().u;
    ret.v = sample1().u;
    return ret;
}

inline Sample3 Sampler::sample3() noexcept
{
    Sample3 ret;
    ret.u = sample1().u;
    ret.v = sample1().u;
    ret.w = sample1().u;
    return ret;
}

inline Sample4 Sampler::sample4() noexcept
{
    Sample4 ret;
    ret.u = sample1().u;
    ret.v = sample1().u;
    ret.w = sample1().u;
    ret.r = sample1().u;
    return ret;
}

inline Sample5 Sampler::sample5() noexcept
{
    Sample5 ret;
    ret.u = sample1().u;
    ret.v = sample1().u;
    ret.w = sample1().u;
    ret.r = sample1().u;
    ret.s = sample1().u;
    return ret;
}

template<int N>
SampleN<N> Sampler::SampleN() noexcept
{
    ::agz::tracer::SampleN<N> ret;
    for(int i = 0; i < N; ++i)
        ret.u[i] = sample1().u;
    return ret;
}

AGZ_TRACER_END
