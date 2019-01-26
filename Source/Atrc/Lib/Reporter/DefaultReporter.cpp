#include <iostream>

#include <Atrc/Lib/Reporter/DefaultReporter.h>

namespace Atrc
{
    
void DefaultReporter::Start()
{
    clock_.Restart();
    std::cout << "Start rendering..." << std::endl;
}

void DefaultReporter::End()
{
    std::cout << "Complete rendering...Total time: "
              << clock_.Milliseconds() / 1000.0 << "s" << std::endl;
}

void DefaultReporter::Report([[maybe_unused]] const Film &film, std::optional<Real> percent)
{
    if(percent)
        std::cout << "Progress: " << *percent << std::endl;
}

void DefaultReporter::Message(const Str8 &msg)
{
    std::cout << msg.ToStdString() << std::endl;
}

} // namespace Atrc
