#pragma once

#include <agz/editor/resource/pool/image_resource_pool.h>

AGZ_EDITOR_BEGIN

using Texture3DWidget        = ResourceWidget       <tracer::Texture3D>;
using Texture3DWidgetCreator = ResourceWidgetCreator<tracer::Texture3D>;
using Texture3DWidgetFactory = ResourceWidgetFactory<tracer::Texture3D>;
using Texture3DPanel         = ResourcePanel        <tracer::Texture3D>;
using Texture3DReference     = ResourceReference    <tracer::Texture3D>;
using Texture3DInPool        = ResourceInPool       <tracer::Texture3D>;
using Texture3DSlot          = ResourceSlot         <tracer::Texture3D>;

using Texture3DPool = ImageResourcePool<tracer::Texture3D>;

AGZ_EDITOR_END
