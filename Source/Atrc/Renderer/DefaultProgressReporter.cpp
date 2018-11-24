#include <iostream>
#include <Atrc/Renderer/DefaultProgressReporter.h>

AGZ_NS_BEG(Atrc)

void DefaultProgressReporter::Begin()
{
    lastProgress_ = -1.0;
    clock_.Restart();

    std::cout << "Start rendering..." << std::endl;
}

void DefaultProgressReporter::End()
{
    std::cout << "Complete rendering...Total time: " << clock_.Milliseconds() / 1000.0 << "s." << std::endl;
}

void DefaultProgressReporter::Report(Real percent)
{
    std::lock_guard<std::mutex> lk(outMut);
    if(percent <= lastProgress_)
        return;

    std::cout << "Progress: " << percent << "%" << std::endl;
    lastProgress_ = percent;
}

void DefaultProgressReporter::Message(const Str8 &msg)
{
    std::lock_guard<std::mutex> lk(outMut);
    std::cout << msg.ToStdString() << std::endl;
}

AGZ_NS_END(Atrc)
