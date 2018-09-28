#pragma once

#include "../Common.h"
#include "Sample.h"

AGZ_NS_BEG(Atrc)

class GlobalSampleGenerator1D
{
public:

    using SampleType = Sample1D;

    SampleType Next()
    {
        return Random::Uniform(Real(0), Real(1));
    }
};

class GlobalSampleGenerator2D
{
public:

    using SampleType = Sample2D;

    SampleType Next()
    {
        return {
            Random::Uniform(Real(0), Real(1)),
            Random::Uniform(Real(0), Real(1))
        };
    }
};

AGZ_NS_END(Atrc)
