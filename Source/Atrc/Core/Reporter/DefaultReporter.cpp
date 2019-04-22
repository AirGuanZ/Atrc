#include <iostream>

#include <Atrc/Core/Reporter/DefaultReporter.h>

namespace Atrc
{
    
void DefaultReporter::Start()
{
    pbar_ = std::make_unique<AGZ::ProgressBarF>(80, '=');
}

void DefaultReporter::End()
{
    if(pbar_)
    {
        pbar_->Done();
        pbar_ = nullptr;
    }
}

void DefaultReporter::Report([[maybe_unused]] const Film &film, std::optional<Real> percent)
{
    if(percent)
    {
        pbar_->SetPercent(*percent);
        pbar_->Display();
    }
}

void DefaultReporter::Message(std::string_view msg)
{
    std::cout << msg << std::endl;
}

} // namespace Atrc
