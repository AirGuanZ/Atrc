#include "./medium_void.h"

AGZ_TRACER_BEGIN

Medium *create_void(
    obj::Object::CustomedFlag customed_flag,
    Arena &arena)
{
    return arena.create<VoidMedium>(customed_flag);
}

AGZT_IMPLEMENTATION(Medium, VoidMedium, "void")

AGZ_TRACER_END
