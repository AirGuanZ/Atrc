#pragma once

#include <agz/tracer/core/renderer_interactor.h>

AGZ_TRACER_BEGIN

RC<RendererInteractor> create_noout_reporter();

RC<RendererInteractor> create_stdout_reporter();

AGZ_TRACER_END
