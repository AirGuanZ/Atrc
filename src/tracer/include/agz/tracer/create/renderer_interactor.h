#pragma once

#include <agz/tracer/core/renderer_interactor.h>

AGZ_TRACER_BEGIN

std::shared_ptr<RendererInteractor> create_noout_reporter();

std::shared_ptr<RendererInteractor> create_stdout_reporter();

AGZ_TRACER_END
