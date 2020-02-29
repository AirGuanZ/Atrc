#pragma once

#include <agz/editor/resource/pool/name_resource_pool.h>

AGZ_EDITOR_BEGIN

using MediumWidget        = ResourceWidget       <tracer::Medium>;
using MediumWidgetCreator = ResourceWidgetCreator<tracer::Medium>;
using MediumWidgetFactory = ResourceWidgetFactory<tracer::Medium>;
using MediumPanel         = ResourcePanel        <tracer::Medium>;
using MediumReference     = ResourceReference    <tracer::Medium>;
using MediumInPool        = ResourceInPool       <tracer::Medium>;
using MediumSlot          = ResourceSlot         <tracer::Medium>;

using MediumPool = NameResourcePool<tracer::Medium>;

AGZ_EDITOR_END
