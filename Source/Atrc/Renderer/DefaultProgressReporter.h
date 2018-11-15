#pragma once

#include <iostream>
#include <mutex>

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class DefaultProgressReporter : public ProgressReporter
{
    std::mutex outMut;
    Real lastProgress_ = -1.0;

public:

    void Report(Real percent) override
    {
        std::lock_guard<std::mutex> lk(outMut);
        if(percent <= lastProgress_)
            return;

        std::cout << "Progress: " << percent << "%" << std::endl;
        lastProgress_ = percent;
    }

    void Message(const Str8 &msg) override
    {
        std::lock_guard<std::mutex> lk(outMut);
        std::cout << msg.ToStdString() << std::endl;
    }
};

AGZ_NS_END(Atrc)
