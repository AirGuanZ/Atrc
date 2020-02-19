#pragma once

#include <agz/tracer/factory/factory.h>
#include <agz/tracer/factory/raw/reporter.h>

AGZ_TRACER_FACTORY_BEGIN

void initialize_reporter_factory(Factory<ProgressReporter> &factory);

AGZ_TRACER_FACTORY_END
