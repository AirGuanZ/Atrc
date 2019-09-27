#pragma once

#include <agz/tracer/core/aggregate.h>

AGZ_TRACER_BEGIN

Aggregate *create_entity_bvh(
    obj::Object::CustomedFlag customed_flag,
    int max_leaf_size,
    Arena &arena);

Aggregate *create_native_aggregate(
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

AGZ_TRACER_END
