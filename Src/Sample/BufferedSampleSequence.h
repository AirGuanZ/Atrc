#pragma once

#include "../Common.h"
#include "SampleSequence.h"

AGZ_NS_BEG(Atrc)

template<typename SampleType_>
class BufferedSampleSequence
    : ATRC_IMPLEMENTS SampleSequence<SampleType_>,
      ATRC_PROPERTY AGZ::Uncopiable
{
    std::vector<SampleType_> buffer_;
    uint32_t restCount_;
    SampleSequence<SampleType_> *basic_;

    void Realloc()
    {
        AGZ_ASSERT(basic_);
        restCount_ = static_cast<uint32_t>(buffer_.size());
        basic_->NextN(restCount_, buffer_.data());
    }

public:

    using SampleType = SampleType_;

    explicit BufferedSampleSequence(uint32_t bufferSize = 32)
        : buffer_(bufferSize), restCount_(0), basic_(nullptr)
    {
        AGZ_ASSERT(bufferSize);
    }

    explicit BufferedSampleSequence(
        SampleSequence<SampleType> &basic, uint32_t bufferSize = 32)
        : buffer_(bufferSize), restCount_(bufferSize), basic_(&basic)
    {
        AGZ_ASSERT(bufferSize);
        basic.NextN(bufferSize, buffer_.data());
    }

    SampleType Next() override
    {
        if(!restCount_)
            Realloc();
        AGZ_ASSERT(buffer_.size() >= restCount_ && restCount_);
        return buffer_[--restCount_];
    }

    void NextN(uint32_t n, SampleType *output) override
    {
        // IMPROVE
        for(uint32_t i = 0; i < n; ++i)
            output[i] = Next();
    }
};

AGZ_NS_END(Atrc)
