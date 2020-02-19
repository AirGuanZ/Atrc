#pragma once

#include <cassert>
#include <iostream>
#include <sstream>
#include <string_view>

#include <spdlog/spdlog.h>

#include <agz/tracer/common.h>

AGZ_TRACER_BEGIN

#define AGZ_INFO  (spdlog::info )
#define AGZ_ERROR (spdlog::error)
#define AGZ_WARN  (stdlog::warn )

AGZ_TRACER_END
