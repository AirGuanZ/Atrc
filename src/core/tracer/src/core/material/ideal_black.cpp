#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/material.h>
#include <agz/utility/misc.h>

#include "ideal_black.h"

AGZ_TRACER_BEGIN

std::shared_ptr<Material> create_ideal_black()
{
    return std::make_shared<IdealBlack>();
}

AGZ_TRACER_END
