#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/material.h>

#include "ideal_black.h"

AGZ_TRACER_BEGIN

Material *create_ideal_black(
    obj::Object::CustomedFlag customed_flag,
    Arena &arena)
{
    return arena.create<IdealBlack>(customed_flag);
}

AGZT_IMPLEMENTATION(Material, IdealBlack, "ideal_black")

AGZ_TRACER_END
