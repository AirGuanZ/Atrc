#pragma once

#include <agz/tracer/core/aggregate.h>

AGZ_TRACER_BEGIN

std::shared_ptr<Aggregate> create_entity_bvh(
    int max_leaf_size);

std::shared_ptr<Aggregate> create_native_aggregate();

AGZ_TRACER_END
