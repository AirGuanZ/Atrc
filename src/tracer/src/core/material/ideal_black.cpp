#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/material.h>
#include <agz/utility/misc.h>

#include "ideal_black.h"

AGZ_TRACER_BEGIN

RC<Material> create_ideal_black()
{
    return newRC<IdealBlack>();
}

AGZ_TRACER_END
