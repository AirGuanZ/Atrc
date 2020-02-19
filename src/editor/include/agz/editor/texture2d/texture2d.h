#pragma once

#include <agz/editor/resource/image_resource_pool.h>

AGZ_EDITOR_BEGIN

using Texture2DWidget        = ResourceWidget       <tracer::Texture2D>;
using Texture2DWidgetCreator = ResourceWidgetCreator<tracer::Texture2D>;
using Texture2DWidgetFactory = ResourceWidgetFactory<tracer::Texture2D>;
using Texture2DPanel         = ResourcePanel        <tracer::Texture2D>;
using Texture2DReference     = ResourceReference    <tracer::Texture2D>;
using Texture2DInPool        = ResourceInPool       <tracer::Texture2D>;
using Texture2DSlot          = ResourceSlot         <tracer::Texture2D>;

using Texture2DPool = ImageResourcePool<tracer::Texture2D>;

AGZ_EDITOR_END
