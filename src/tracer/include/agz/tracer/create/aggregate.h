#pragma once

#include <agz/tracer/core/aggregate.h>

AGZ_TRACER_BEGIN

RC<Aggregate> create_entity_bvh(
    int max_leaf_size);

RC<Aggregate> create_entity_bvh_embree(
    int max_leaf_size);

RC<Aggregate> create_entity_bvh_noembree(
    int max_leaf_size);

RC<Aggregate> create_native_aggregate();

AGZ_TRACER_END
