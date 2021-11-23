#pragma once

#include <agz/tracer/common.h>

AGZ_TRACER_BEGIN

template<typename T>
struct Reservoir
{
    T data;

    int  M    = 0;
    real W    = 0;
    real wsum = 0;

    void clear()
    {
        wsum = 0;
        W    = 0;
        M    = 0;
    }

    bool update(const T &new_data, real new_weight, real rnd)
    {
        wsum += new_weight;
        ++M;
        if(new_weight > 0 && rnd <= new_weight / wsum)
        {
            data = new_data;
            return true;
        }
        return false;
    }

    void merge(const Reservoir<T> &other, real p_hat, real rnd)
    {
        const int old_M = M;
        update(other.data, p_hat * other.W * other.M, rnd);
        M = old_M + other.M;
    }
};

AGZ_TRACER_END
