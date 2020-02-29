#pragma once

#include <agz/editor/resource/pool/image_resource_pool.h>

AGZ_EDITOR_BEGIN

using EntityWidget        = ResourceWidget       <tracer::Entity>;
using EntityWidgetCreator = ResourceWidgetCreator<tracer::Entity>;
using EntityWidgetFactory = ResourceWidgetFactory<tracer::Entity>;
using EntityPanel         = ResourcePanel        <tracer::Entity>;
using EntitySlot          = ResourceSlot         <tracer::Entity>;

AGZ_EDITOR_END
