#pragma once

#include "../Common.h"
#include "Sample.h"

AGZ_NS_BEG(Atrc)

template<typename SampleType_>
ATRC_INTERFACE SampleSequence
{
public:

    using SampleType = SampleType_;

    virtual ~SampleSequence() = default;

    virtual SampleType Next() = 0;

    virtual void NextN(uint32_t n, SampleType *output) = 0;
};

template<typename SampleGenerator>
class SampleGenerator2Sequence
    : ATRC_IMPLEMENTS SampleSequence<typename SampleGenerator::SampleType>,
      ATRC_PROPERTY AGZ::Uncopiable
{
    SampleGenerator &generator_;

public:

    using SampleType = typename SampleGenerator::SampleType;

    explicit SampleGenerator2Sequence(SampleGenerator &generator)
        : generator_(generator)
    {
        
    }

    SampleType Next() override
    {
        return generator_.Next();
    }

    void NextN(uint32_t n, SampleType *output) override
    {
        AGZ_ASSERT(output);
        for(uint32_t i = 0; i < n; ++i)
            output[i] = generator_.Next();
    }
};

using SampleSeq1D = SampleSequence<Sample1D>;
using SampleSeq2D = SampleSequence<Sample2D>;

AGZ_NS_END(Atrc)
