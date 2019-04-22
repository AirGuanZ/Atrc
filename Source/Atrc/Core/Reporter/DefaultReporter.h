#pragma once

#include <AGZUtils/Utils/Console.h>
#include <AGZUtils/Utils/Time.h>

#include <Atrc/Core/Core/Reporter.h>

namespace Atrc
{
    
class DefaultReporter : public Reporter
{
    std::unique_ptr<AGZ::ProgressBarF> pbar_;

public:

    void Start() override;

    void End() override;

    void Report(const Film &film, std::optional<Real> percent) override;

    void Message(std::string_view msg) override;
};

} // namespace Atrc
