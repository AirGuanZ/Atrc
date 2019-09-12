#pragma once

#include <agz/utility/common/common.h>
#include <agz/utility/alloc.h>

#define AGZ_TRACER_BEGIN namespace agz::tracer {
#define AGZ_TRACER_END   }

#define AGZ_ANONYMOUS_NAMESPACE_BEGIN namespace {
#define AGZ_ANONYMOUS_NAMESPACE_END   }

AGZ_TRACER_BEGIN

using Arena = alloc::releaser_t;

class ObjectConstructionException : public std::runtime_error
{
public:

    using runtime_error::runtime_error;
};

AGZ_TRACER_END
