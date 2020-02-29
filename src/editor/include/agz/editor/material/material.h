#pragma once

#include <agz/editor/resource/pool/image_resource_pool.h>

AGZ_EDITOR_BEGIN

using MaterialWidget        = ResourceWidget       <tracer::Material>;
using MaterialWidgetCreator = ResourceWidgetCreator<tracer::Material>;
using MaterialWidgetFactory = ResourceWidgetFactory<tracer::Material>;
using MaterialPanel         = ResourcePanel        <tracer::Material>;
using MaterialReference     = ResourceReference    <tracer::Material>;
using MaterialInPool        = ResourceInPool       <tracer::Material>;
using MaterialSlot          = ResourceSlot         <tracer::Material>;

using MaterialPool = ImageResourcePool<tracer::Material>;

AGZ_EDITOR_END
