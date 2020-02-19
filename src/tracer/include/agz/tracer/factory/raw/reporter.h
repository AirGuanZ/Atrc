#pragma once

#include <agz/tracer/core/reporter.h>

AGZ_TRACER_BEGIN

std::shared_ptr<ProgressReporter> create_noout_reporter();

std::shared_ptr<ProgressReporter> create_stdout_reporter();

AGZ_TRACER_END
