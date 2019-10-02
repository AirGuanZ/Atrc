#pragma once

#include <agz/tracer/core/medium.h>
#include <agz/tracer/utility/config.h>

AGZ_TRACER_BEGIN

struct MediumInterface
{
    std::shared_ptr<const Medium> in;
    std::shared_ptr<const Medium> out;
};

AGZ_TRACER_END
