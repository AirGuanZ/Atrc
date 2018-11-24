#pragma once

#include <mutex>

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class DefaultProgressReporter : public ProgressReporter
{
    std::mutex outMut;
    Real lastProgress_ = -1.0;

    AGZ::Clock clock_;

public:

    void Begin() override;

    void End() override;

    void Report(Real percent) override;

    void Message(const Str8 &msg) override;
};

AGZ_NS_END(Atrc)
