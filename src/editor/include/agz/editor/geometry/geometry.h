#pragma once

#include <agz/editor/resource/pool/name_resource_pool.h>

AGZ_EDITOR_BEGIN

using GeometryWidget        = ResourceWidget<tracer::Geometry>;
using GeometryWidgetCreator = ResourceWidgetCreator<tracer::Geometry>;
using GeometryWidgetFactory = ResourceWidgetFactory<tracer::Geometry>;
using GeometryPanel         = ResourcePanel<tracer::Geometry>;
using GeometryReference     = ResourceReference<tracer::Geometry>;
using GeometryInPool        = ResourceInPool<tracer::Geometry>;
using GeometrySlot          = ResourceSlot<tracer::Geometry>;

using GeometryPool = NameResourcePool<tracer::Geometry>;

AGZ_EDITOR_END
