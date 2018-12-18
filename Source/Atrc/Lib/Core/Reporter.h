#pragma once

#include <Atrc/Lib/Core/Common.h>

namespace Atrc
{

class Reporter
{
public:

    virtual ~Reporter() = default;

    virtual void Start() = 0;

    virtual void End() = 0;

    virtual void Report(const Film &film, Option<Real> percent) = 0;

    virtual void Message(const Str8 &msg) = 0;
};

} // namespace Atrc
