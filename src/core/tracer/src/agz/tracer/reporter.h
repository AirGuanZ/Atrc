#pragma once

#include <agz/tracer/core/reporter.h>

AGZ_TRACER_BEGIN

ProgressReporter *create_noout_reporter(
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

ProgressReporter *create_stdout_reporter(
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

AGZ_TRACER_END
