#pragma once

#include <agz/editor/resource/resource.h>
#include <agz/editor/ui/utility/flow_layout.h>

AGZ_EDITOR_BEGIN

using EnvirLightWidget        = ResourceWidget<tracer::EnvirLight>;
using EnvirLightWidgetCreator = ResourceWidgetCreator<tracer::EnvirLight>;
using EnvirLightWidgetFactory = ResourceWidgetFactory<tracer::EnvirLight>;
using EnvirLightSlot          = ResourceSlot<tracer::EnvirLight>;

AGZ_EDITOR_END
