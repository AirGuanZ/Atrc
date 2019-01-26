#pragma once

#include <AGZUtils/Utils/Time.h>

#include <Atrc/Lib/Core/Reporter.h>

namespace Atrc
{
    
class DefaultReporter : public Reporter
{
    AGZ::Clock clock_;

public:

    void Start() override;

    void End() override;

    void Report(const Film &film, std::optional<Real> percent) override;

    void Message(const Str8& msg) override;
};

} // namespace Atrc
