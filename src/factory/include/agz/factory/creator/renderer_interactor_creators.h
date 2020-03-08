#pragma once

#include <agz/factory/factory.h>
#include <agz/tracer/create/renderer_interactor.h>

AGZ_TRACER_FACTORY_BEGIN

void initialize_renderer_interactor_factory(Factory<RendererInteractor> &factory);

AGZ_TRACER_FACTORY_END
